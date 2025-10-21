/* pickline.c — печать выбранной строки из текстового файла
   Построение индекса (offset/len) по '\n' с использованием open/read/lseek.
   Ввод номера строки с клавиатуры; 0 — выход.
   Опция командной строки -t выводит таблицу смещений/длин (для отладки).
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

typedef struct {
    off_t  off;   /* смещение начала строки от начала файла */
    size_t len;   /* длина строки БЕЗ символа '\n' */
} Line;

typedef struct {
    Line  *a;
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

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Использование: %s [-t] <файл>\n", argv[0]);
        return 2;
    }

    int show_table = 0;
    const char *path = NULL;
    if (argc == 3 && strcmp(argv[1], "-t") == 0) {
        show_table = 1;
        path = argv[2];
    } else {
        path = argv[1];
    }

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

    if (show_table) {
        printf("Таблица строк (всего %zu):\n", idx.n);
        for (size_t i = 0; i < idx.n; ++i) {
            printf("  %6zu: off=%lld len=%zu\n",
                   i + 1, (long long)idx.a[i].off, idx.a[i].len);
        }
    }

    /* Цикл запросов */
    char inbuf[128];
    for (;;) {
        printf("Введите номер строки (0 — выход, 1..%zu): ", idx.n);
        if (!fgets(inbuf, sizeof(inbuf), stdin)) break;

        char *endp = NULL;
        long n = strtol(inbuf, &endp, 10);
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
