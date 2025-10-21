/* pickline_mmap.c — выбор и печать строки из текстового файла через mmap.
   - Строит индекс (off,len) по '\n', сканируя mmapped-память.
   - На ввод номера строки отводится 5 секунд (select на stdin).
   - При таймауте печатает весь файл построчно и выходит.
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
#include <sys/mman.h>
#include <sys/select.h>
#include <errno.h>

typedef struct { off_t off; size_t len; } Line;

typedef struct {
    Line  *a;
    size_t n, cap;
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

/* Печать строки из mmapped-буфера (добавляем \n сами). */
static void print_line(const char *base, const Line *L) {
    /* Уберём возможный CR перед LF (Windows CRLF), чтобы вывод был “чистым” */
    size_t n = L->len;
    if (n && base[L->off + n - 1] == '\r') n--;
    (void)fwrite(base + L->off, 1, n, stdout);
    fputc('\n', stdout);
}

/* Печать всего файла построчно по индексу. */
static void print_all(const char *base, const LineVec *idx) {
    for (size_t i = 0; i < idx->n; ++i) print_line(base, &idx->a[i]);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Использование: %s <файл>\n", argv[0]);
        return 2;
    }
    const char *path = argv[1];

    /* 1) Открыть и отобразить файл */
    int fd = open(path, O_RDONLY);
    if (fd < 0) die("open");

    struct stat st;
    if (fstat(fd, &st) < 0) die("fstat");

    off_t fsz = st.st_size;
    if (fsz == 0) {  /* пустой файл */
        printf("Файл пуст.\n");
        close(fd);
        return 0;
    }

    void *map = mmap(NULL, (size_t)fsz, PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) die("mmap");
    const char *base = (const char*)map;

    /* 2) Построить индекс (off,len) по '\n' */
    LineVec idx = {0};
    off_t line_start = 0;
    for (off_t i = 0; i < fsz; ++i) {
        if (base[i] == '\n') {
            off_t line_end = i;                    /* позиция LF */
            size_t len = (size_t)(line_end - line_start); /* без LF */
            lv_push(&idx, line_start, len);
            line_start = i + 1;
        }
    }
    /* Хвост без завершающего LF */
    if (line_start < fsz) {
        size_t len = (size_t)(fsz - line_start);
        lv_push(&idx, line_start, len);
    }

    /* 3) Цикл запросов с таймаутом 5 сек */
    char inbuf[128];
    for (;;) {
        printf("Введите номер строки (0 — выход, 1..%zu) [5 секунд]: ", idx.n);
        fflush(stdout);

        fd_set rfds; FD_ZERO(&rfds); FD_SET(STDIN_FILENO, &rfds);
        struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
        int sel = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);

        if (sel == 0) {
            printf("\nТаймаут. Печатаю всё содержимое файла:\n");
            print_all(base, &idx);
            break;
        } else if (sel < 0) {
            if (errno == EINTR) continue;
            die("select");
        }

        if (!fgets(inbuf, sizeof(inbuf), stdin)) break;
        char *endp = NULL;
        long n = strtol(inbuf, &endp, 10);
        if (n == 0) break;
        if (n < 0 || (size_t)n > idx.n) {
            printf("Нет такой строки. В файле %zu строк(и).\n", idx.n);
            continue;
        }
        print_line(base, &idx.a[n - 1]);
        /* и снова ждём 5 секунд следующего ввода */
    }

    /* 4) Чистка */
    free(idx.a);
    munmap((void*)base, (size_t)fsz);
    close(fd);
    return 0;
}
