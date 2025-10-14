/* ===================================================
 * task7.c — Индекс строк + mmap + таймаут ввода 5 секунд
 * Сборка: gcc -std=c11 -Wall -Wextra -O2 task7.c -o task7
 * Запуск: ./task7 path/to/file.txt
 * ---------------------------------------------------
 * Использует mmap для чтения файла (без read/lseek по файлу).
 * По таймауту печатает весь файл и завершает работу.
 * =================================================== */
#define _XOPEN_SOURCE 700
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

typedef struct { off_t offset; size_t len; } line_rec_t;
typedef struct { line_rec_t *data; size_t size, cap; } lines_t;

static void lines_init(lines_t *ls){ ls->data=NULL; ls->size=0; ls->cap=0; }
static void lines_free(lines_t *ls){ free(ls->data); ls->data=NULL; ls->size=ls->cap=0; }
static void lines_push(lines_t *ls, off_t off, size_t len){
    if(ls->size == ls->cap){
        size_t ncap = ls->cap ? ls->cap*2 : 256;
        line_rec_t *nd = (line_rec_t*)realloc(ls->data, ncap*sizeof(*nd));
        if(!nd){ perror("realloc"); exit(1); }
        ls->data = nd; ls->cap = ncap;
    }
    ls->data[ls->size].offset = off;
    ls->data[ls->size].len    = len;
    ls->size++;
}

static volatile sig_atomic_t g_timed_out=0;
static void on_alarm(int sig){ (void)sig; g_timed_out=1; }

static int read_number_with_timeout(unsigned long long *out){
    g_timed_out=0;
    alarm(5);
    char line[256];
    if(!fgets(line, sizeof(line), stdin)){
        if(g_timed_out) return 0; /* timeout/EOF */
        return 0;
    }
    alarm(0);
    char *end=NULL;
    errno=0;
    unsigned long long val = strtoull(line, &end, 10);
    if(errno==ERANGE) return -1;
    while(end && (*end==' '||*end=='\t'||*end=='\n'||*end=='\r')) end++;
    if(end && *end!='\0') return -1;
    *out = val;
    return 1;
}

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr,"Использование: %s <file>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if(fd < 0){ perror("open"); return 1; }

    struct stat st;
    if(fstat(fd, &st) < 0){ perror("fstat"); close(fd); return 1; }
    off_t fsz = st.st_size;

    if(fsz == 0){
        printf("Файл пуст.\n");
        close(fd);
        return 0;
    }

    void *map = mmap(NULL, (size_t)fsz, PROT_READ, MAP_PRIVATE, fd, 0);
    if(map == MAP_FAILED){ perror("mmap"); close(fd); return 1; }
    const char *base = (const char*)map;

    lines_t ls; lines_init(&ls);
    off_t line_start = 0;
    size_t line_len = 0;

    for(off_t i=0; i<fsz; ++i){
        char c = base[i];
        if(c=='\n'){
            lines_push(&ls, line_start, line_len);
            line_start = i+1;
            line_len = 0;
        }else{
            line_len++;
        }
    }
    /* последняя строка без '\n' */
    if(line_start < fsz){
        lines_push(&ls, line_start, (size_t)(fsz - line_start));
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGALRM, &sa, NULL) < 0){
        perror("sigaction"); munmap(map, (size_t)fsz); close(fd); return 1;
    }

    while(1){
        printf("Введите номер строки (1..%zu, 0 — выход) [5 секунд]: ", ls.size);
        fflush(stdout);

        unsigned long long num=0ULL;
        int rc = read_number_with_timeout(&num);
        if(rc == 0){
            printf("\nТаймаут/EOF. Печатаю весь файл и выхожу:\n");
            off_t left = fsz;
            const char *p = base;
            while(left > 0){
                ssize_t chunk = (left > SSIZE_MAX ? SSIZE_MAX : (ssize_t)left);
                ssize_t w = write(STDOUT_FILENO, p, chunk);
                if(w < 0){
                    if(errno == EINTR) continue;
                    perror("write"); break;
                }
                left -= w; p += w;
            }
            munmap(map, (size_t)fsz);
            close(fd);
            return 0;
        }else if(rc < 0){
            fprintf(stderr,"Некорректный ввод. Повторите.\n");
            continue;
        }

        if(num == 0ULL) break;
        if(num < 1ULL || num > (unsigned long long)ls.size){
            fprintf(stderr,"Нет такой строки. Диапазон: 1..%zu\n", ls.size);
            continue;
        }

        line_rec_t rec = ls.data[num-1];
        const char *p = base + rec.offset;

        off_t left = (off_t)rec.len;
        while(left > 0){
            ssize_t chunk = (left > SSIZE_MAX ? SSIZE_MAX : (ssize_t)left);
            ssize_t w = write(STDOUT_FILENO, p, chunk);
            if(w < 0){
                if(errno == EINTR) continue;
                perror("write"); break;
            }
            left -= w; p += w;
        }
        if(write(STDOUT_FILENO, "\n", 1) < 0) perror("write");
    }

    lines_free(&ls);
    munmap(map, (size_t)fsz);
    close(fd);
    return 0;
}
