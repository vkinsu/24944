
#define _XOPEN_SOURCE 700
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/* Запись о строке */
typedef struct {
    off_t  offset;  /* смещение начала строки в файле */
    size_t len;     /* длина строки (без завершающего '\n', если он есть) */
} line_rec_t;

/* Динамический массив записей */
typedef struct {
    line_rec_t *data;
    size_t size;
    size_t cap;
} lines_t;

static void lines_init(lines_t *ls) {
    ls->data = NULL;
    ls->size = 0;
    ls->cap  = 0;
}
static void lines_free(lines_t *ls) {
    free(ls->data);
    ls->data = NULL;
    ls->size = ls->cap = 0;
}
static void lines_push(lines_t *ls, off_t off, size_t len) {
    if (ls->size == ls->cap) {
        size_t ncap = ls->cap ? ls->cap * 2 : 256;
        line_rec_t *nd = (line_rec_t*)realloc(ls->data, ncap * sizeof(*nd));
        if (!nd) { perror("realloc"); exit(1); }
        ls->data = nd;
        ls->cap  = ncap;
    }
    ls->data[ls->size].offset = off;
    ls->data[ls->size].len    = len;
    ls->size++;
}

/* Построить индекс строк: пробегаем файл и фиксируем позиции '\n' */
static void build_index(int fd, lines_t *ls) {
    lines_init(ls);

    const size_t BUF = 1 << 15; /* 32 KiB */
    char *buf = (char*)malloc(BUF);
    if (!buf) { perror("malloc"); exit(1); }

    /* Первая строка начинается с offset = 0 */
    off_t file_pos   = 0; /* абсолютная позиция начала блока в файле */
    off_t line_start = 0; /* смещение начала текущей строки */
    size_t line_len  = 0; /* текущая длина строки без '\n' */

    if (lseek(fd, 0L, SEEK_SET) == (off_t)-1) {
        perror("lseek");
        free(buf);
        exit(1);
    }

    ssize_t r;
    while ((r = read(fd, buf, BUF)) > 0) {
        for (ssize_t i = 0; i < r; ++i) {
            char c = buf[i];
            if (c == '\n') {
                /* Закрываем строку при '\n' */
                lines_push(ls, line_start, line_len);
                line_start = file_pos + i + 1; /* байт после '\n' */
                line_len = 0;
            } else {
                line_len++;
            }
        }
        file_pos += r;

        /* Демонстрация lseek(fd, 0L, 1) из подсказки (эквивалент SEEK_CUR) */
        if (lseek(fd, 0L, SEEK_CUR) == (off_t)-1) {
            perror("lseek(SEEK_CUR)"); free(buf); exit(1);
        }
    }
    if (r < 0) {
        perror("read"); free(buf); exit(1);
    }

    /* Последняя строка без завершающего '\n' — тоже должна попасть в таблицу */
    if ((size_t)(file_pos - line_start) == line_len) {
        if (line_len > 0 || ls->size == 0 ||
            (ls->size > 0 && ls->data[ls->size - 1].offset != line_start)) {
            lines_push(ls, line_start, line_len);
        }
    }

    free(buf);

    /* Вернуться в начало, чтобы потом читать/печатать выбранные строки */
    if (lseek(fd, 0L, SEEK_SET) == (off_t)-1) {
        perror("lseek"); exit(1);
    }
}

/* Печать строки по номеру (1..N): используем lseek+read точной длины */
static int print_line(int fd, const lines_t *ls, size_t lineno) {
    if (lineno == 0 || lineno > ls->size) return -1;
    line_rec_t rec = ls->data[lineno - 1];

    if (lseek(fd, rec.offset, SEEK_SET) == (off_t)-1) {
        perror("lseek");
        return -1;
    }

    if (rec.len == 0) {
        /* Пустая строка: просто вывести перевод строки */
        if (write(STDOUT_FILENO, "\n", 1) < 0) perror("write");
        return 0;
    }

    char *buf = (char*)malloc(rec.len);
    if (!buf) { perror("malloc"); return -1; }

    size_t left = rec.len;
    char *p = buf;
    while (left > 0) {
        ssize_t got = read(fd, p, left);
        if (got < 0) {
            if (errno == EINTR) continue;
            perror("read"); free(buf); return -1;
        }
        if (got == 0) break; /* неожиданно EOF */
        left -= (size_t)got;
        p    += got;
    }

    if (write(STDOUT_FILENO, buf, rec.len) < 0) perror("write");
    if (write(STDOUT_FILENO, "\n", 1) < 0) perror("write");

    free(buf);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <file>\n", argv[0]);
        return 1;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    lines_t ls;
    build_index(fd, &ls);

    /* === ВАЖНО: ВЫВОД ТАБЛИЦЫ ДЛЯ ОТЛАДКИ (как требует задание) === */
    if (ls.size == 0) {
        printf("Таблица строк: (файл пуст)\n");
    } else {
        printf("Таблица строк:\n");
        for (size_t i = 0; i < ls.size; ++i) {
            printf("%zu: %lld, %zu\n",
                   i + 1,
                   (long long)ls.data[i].offset,
                   ls.data[i].len);
        }
    }
    printf("=== Конец таблицы ===\n");

    /* === Основной цикл запросов === */
    while (1) {
        printf("Введите номер строки (1..%zu, 0 — выход): ", ls.size);
        fflush(stdout);

        unsigned long long num = 0ULL;
        if (scanf("%llu", &num) != 1) {
            /* очистка некорректного ввода */
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            fprintf(stderr, "Некорректный ввод. Повторите.\n");
            continue;
        }
        if (num == 0ULL) break;

        if (num < 1ULL || num > (unsigned long long)ls.size) {
            fprintf(stderr, "Нет такой строки. Диапазон: 1..%zu\n", ls.size);
            continue;
        }

        if (print_line(fd, &ls, (size_t)num) != 0) {
            fprintf(stderr, "Ошибка печати строки %llu\n", num);
        }
    }

    lines_free(&ls);
    close(fd);
    return 0;
}
