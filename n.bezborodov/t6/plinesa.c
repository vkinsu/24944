/* pickline_timeout.c — печать выбранной строки из текстового файла
   Строит индекс (off,len) через open/read/lseek/close.
   На ввод номера строки отводится 5 секунд. Если таймаут — печатаем весь файл и выходим.
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
#include <sys/select.h>
#include <errno.h>

typedef struct {
    off_t  off;   /* смещение начала строки */
    size_t len;   /* длина строки БЕЗ '\n' */
} Line;

typedef struct {
    Line  *a;
    size_t n;
    size_t cap;
} LineVec;

static void die(const char *msg) { perror(msg); exit(1); }

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

static void print_line_by_index(int fd, const Line *L) {
    if (lseek(fd, L->off, SEEK_SET) == (off_t)-1) die("lseek");
    char *buf = (char*)malloc(L->len ? L->len : 1);
    if (!buf) die("malloc");
    size_t got = 0;
    while (got < L->len) {
        ssize_t r = read(fd, buf + got, L->len - got);
        if (r < 0) { free(buf); die("read line"); }
        if (r == 0) break;
        got += (size_t)r;
    }
    /* Печатаем ровно длину строки, затем '\n' */
    printf("%.*s\n", (int)got, buf);
    free(buf);
}

static void print_whole_file(int fd, const LineVec *idx) {
    for (size_t i = 0; i < idx->n; ++i) {
        print_line_by_index(fd, &idx->a[i]);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Использование: %s <файл>\n", argv[0]);
        return 2;
    }
    const char *path = argv[1];

    int fd = open(path, O_RDONLY);
    if (fd < 0) die("open");

    LineVec idx = {0};
    /* Построим индекс */
    const size_t BUFSZ = 4096;
    char *buf = (char*)malloc(BUFSZ);
    if (!buf) die("malloc");

    off_t file_pos = 0;
    off_t line_start = 0;
    ssize_t rd;
    while ((rd = read(fd, buf, BUFSZ)) > 0) {
        for (ssize_t i = 0; i < rd; ++i) {
            if (buf[i] == '\n') {
                off_t line_end = file_pos + i; /* позиция '\n' */
                size_t len = (size_t)(line_end - line_start);
                lv_push(&idx, line_start, len);
                line_start = line_end + 1;
            }
        }
        file_pos += rd;
    }
    if (rd < 0) die("read");

    if (line_start < file_pos) { /* последняя строка без '\n' */
        size_t len = (size_t)(file_pos - line_start);
        lv_push(&idx, line_start, len);
    }

    /* Основной цикл: таймаут 5 секунд на ввод */
    char inbuf[128];
    for (;;) {
        printf("Введите номер строки (0 — выход, 1..%zu) [5 секунд]: ", idx.n);
        fflush(stdout);

        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int sel = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
        if (sel == 0) {
            /* Таймаут — печатаем весь файл и уходим */
            printf("\nТаймаут. Печатаю всё содержимое файла:\n");
            print_whole_file(fd, &idx);
            break;
        } else if (sel < 0) {
            if (errno == EINTR) continue; /* прерван сигналом — повторим */
            die("select");
        }

        /* Данные готовы — читаем строку */
        if (!fgets(inbuf, sizeof(inbuf), stdin)) break;

        char *endp = NULL;
        long n = strtol(inbuf, &endp, 10);
        if (n == 0) break;
        if (n < 0 || (size_t)n > idx.n) {
            printf("Нет такой строки. В файле %zu строк(и).\n", idx.n);
            continue;
        }

        print_line_by_index(fd, &idx.a[n - 1]);
        /* продолжаем цикл — снова таймер на 5 секунд для следующего запроса */
    }

    free(buf);
    free(idx.a);
    close(fd);
    return 0;
}
