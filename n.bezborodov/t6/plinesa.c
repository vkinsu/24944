/* pickline.c — печать выбранной строки из текстового файла
   Построение индекса (offset/len) по '\n' с использованием open/read/lseek.
   Ввод номера строки с клавиатуры; 0 — выход.
   Построение таблицы смещений и длин строк для каждой строки файла.
   Таймаут на ввод — 5 секунд для первого ввода.
*/

#define _XOPEN_SOURCE 600
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>

typedef struct {
    off_t off;   /* смещение начала строки от начала файла */
    size_t len;  /* длина строки БЕЗ символа '\n' */
} Line;

typedef struct {
    Line *a;
    size_t n;
    size_t cap;
} LineVec;

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

static void lv_init(LineVec *v) { v->a = NULL; v->n = v->cap = 0; }

static void lv_push(LineVec *v, off_t off, size_t len) {
    if (v->n == v->cap) {
        size_t ncap = v->cap ? v->cap * 2 : 128;
        Line *na = (Line*)realloc(v->a, ncap * sizeof(Line));
        if (!na) die("realloc");
        v->a = na; v->cap = ncap;
    }
    v->a[v->n].off = off;
    v->a[v->n].len = len;
    v->n++;
}

// Переменная для отслеживания, был ли сделан первый выбор
static int first_choice = 1;

// Имя файла, переданное в командной строке
static const char *path;

// Функция обработчик для alarm (тайм-аут)
void handle_alarm(int sig) {
    if (first_choice) {
        printf("\nВремя истекло! Печатаем весь файл:\n");

        // Печатаем весь файл
        int fd = open(path, O_RDONLY);  // Используем переменную path
        if (fd < 0) die("open");

        const size_t BUFSZ = 4096;
        char *buf = (char*)malloc(BUFSZ);
        if (!buf) die("malloc");

        ssize_t rd;
        while ((rd = read(fd, buf, BUFSZ)) > 0) {
            fwrite(buf, 1, rd, stdout);
        }

        free(buf);
        close(fd);

        // Завершаем программу
        exit(0);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Использование: %s <файл>\n", argv[0]);
        return 2;
    }

    path = argv[1];  // Сохраняем имя файла в глобальной переменной
    
    int fd = open(path, O_RDONLY);
    if (fd < 0) die("open");

    LineVec idx; lv_init(&idx);

    /* Однопроходное построение индекса */
    const size_t BUFSZ = 4096;
    char *buf = (char*)malloc(BUFSZ);
    if (!buf) die("malloc");

    off_t file_pos = 0;        /* позиция начала буфера в файле */
    off_t line_start = 0;      /* смещение начала текущей (набираемой) строки */
    ssize_t rd;

    while ((rd = read(fd, buf, BUFSZ)) > 0) {
        for (ssize_t i = 0; i < rd; ++i) {
            if (buf[i] == '\n') {
                off_t line_end_off = file_pos + i;               /* позиция '\n' */
                size_t len = (size_t)(line_end_off - line_start);/* без '\n' */
                lv_push(&idx, line_start, len);
                line_start = line_end_off + 1;                   /* следом после '\n' */
            }
        }
        file_pos += rd;
    }
    if (rd < 0) die("read");

    /* Если файл не заканчивается '\n', добавим последнюю строку */
    if (line_start < file_pos) {
        size_t len = (size_t)(file_pos - line_start);
        lv_push(&idx, line_start, len);
    }

    /* Выводим таблицу смещений и длин строк для каждой строки */
    printf("Таблица строк (всего %zu):\n", idx.n);
    for (size_t i = 0; i < idx.n; ++i) {
        printf("  %6zu: off=%lld len=%zu\n", i + 1, (long long)idx.a[i].off, idx.a[i].len);
    }

    // Регистрация обработчика сигнала для alarm
    signal(SIGALRM, handle_alarm);

    /* Таймаут на первый ввод (5 секунд) */
    for (;;) {
        if (first_choice) {
            // Сообщение о 5 секундах на первый ввод
            printf("\nУ вас есть 5 секунд. ");
        }
        printf("Введите номер строки (0 — выход, 1..%zu): ", idx.n);

        // Устанавливаем таймер на 5 секунд для первого ввода
        if (first_choice) {
            alarm(5);  // Таймер 5 секунд
        }

        char inbuf[128];
        if (!fgets(inbuf, sizeof(inbuf), stdin)) break;

        // Останавливаем alarm, если пользователь успел ввести
        if (first_choice) {
            first_choice = 0; // После первого ввода таймер больше не нужен
            alarm(0);         // Отключаем таймер
        }

        // Проверяем ввод на корректность
        char *endp = NULL;
        long n = strtol(inbuf, &endp, 10);

        // Проверка, что весь ввод был числом и нет неверных символов
        if (*endp != '\0' && *endp != '\n') {
            printf("Ошибка: введены неверные символы. Пожалуйста, введите корректное число.\n");
            continue;
        }

        if (n == 0) break;
        if (n < 0 || (size_t)n > idx.n) {
            printf("Нет такой строки. В файле %zu строк(и).\n", idx.n);
            continue;
        }

        Line L = idx.a[n - 1];

        /* Позиционируемся на начало строки */
        if (lseek(fd, L.off, SEEK_SET) == (off_t)-1) die("lseek");

        /* Читаем ровно длину строки */
        char *linebuf = (char*)malloc(L.len + 1);
        if (!linebuf) die("malloc");
        size_t got = 0;
        while (got < L.len) {
            ssize_t r = read(fd, linebuf + got, L.len - got);
            if (r < 0) { free(linebuf); die("read line"); }
            if (r == 0) break; /* неожиданный EOF */
            got += (size_t)r;
        }
        linebuf[L.len] = '\0';

        /* Печатаем через printf (строка может быть без завершающего '\n') */
        printf("%s\n", linebuf);
        free(linebuf);
    }

    free(buf);
    free(idx.a);
    close(fd);
    return 0;
}
