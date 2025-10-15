/* ===========================================================
 * task5.c — Таблица поиска строк (open/read/lseek) + печать таблицы
 * Сборка: gcc -std=c11 -Wall -Wextra -O2 task5.c -o task5
 * Запуск: ./task5 path/to/file.txt
 *
 * Поведение:
 * 1) Анализирует файл, строит таблицу (offset,len) каждой строки.
 * 2) Печатает таблицу в виде: № | offset | len
 * 3) Далее запрашивает номера строк и печатает соответствующие строки.
 *    Ввод 0 — завершение.
 * =========================================================== */
#define _XOPEN_SOURCE 700
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
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
    if(ls->size==ls->cap){
        size_t ncap = ls->cap? ls->cap*2 : 256;
        line_rec_t *nd = (line_rec_t*)realloc(ls->data, ncap*sizeof(*nd));
        if(!nd){ perror("realloc"); exit(1); }
        ls->data = nd; ls->cap = ncap;
    }
    ls->data[ls->size].offset = off;
    ls->data[ls->size].len    = len;
    ls->size++;
}

static void build_index(int fd, lines_t *ls){
    lines_init(ls);
    const size_t BUF = 1<<15;
    char *buf = (char*)malloc(BUF);
    if(!buf){ perror("malloc"); exit(1); }

    if(lseek(fd, 0L, SEEK_SET)==(off_t)-1){ perror("lseek"); free(buf); exit(1); }

    off_t file_pos=0, line_start=0;
    size_t line_len=0;

    ssize_t r;
    while((r=read(fd, buf, BUF))>0){
        for(ssize_t i=0;i<r;++i){
            if(buf[i]=='\n'){
                lines_push(ls, line_start, line_len);
                line_start = file_pos + i + 1;
                line_len = 0;
            }else{
                line_len++;
            }
        }
        file_pos += r;
    }
    if(r<0){ perror("read"); free(buf); exit(1); }

    if(line_start < file_pos){
        lines_push(ls, line_start, (size_t)(file_pos - line_start));
    }
    free(buf);

    if(lseek(fd, 0L, SEEK_SET)==(off_t)-1){ perror("lseek"); exit(1); }
}

static void print_table(const lines_t *ls){
    printf("Таблица строк (всего %zu):\n", ls->size);
    printf("%-6s %-12s %-8s\n", "№", "offset", "len");
    for(size_t i=0;i<ls->size;++i){
        printf("%-6zu %-12lld %-8zu\n", i+1, (long long)ls->data[i].offset, ls->data[i].len);
    }
}

static int print_line(int fd, const lines_t *ls, size_t lineno){
    if(lineno==0 || lineno>ls->size) return -1;
    line_rec_t rec = ls->data[lineno-1];

    if(lseek(fd, rec.offset, SEEK_SET)==(off_t)-1){ perror("lseek"); return -1; }
    if(rec.len==0){
        if(write(STDOUT_FILENO, "\n", 1)<0) perror("write");
        return 0;
    }

    char *buf=(char*)malloc(rec.len);
    if(!buf){ perror("malloc"); return -1; }

    size_t left=rec.len; char *p=buf;
    while(left>0){
        ssize_t got = read(fd, p, left);
        if(got<0){
            if(errno==EINTR) continue;
            perror("read"); free(buf); return -1;
        }
        if(got==0) break;
        left -= (size_t)got; p += got;
    }

    if(write(STDOUT_FILENO, buf, rec.len)<0) perror("write");
    if(write(STDOUT_FILENO, "\n", 1)<0) perror("write");
    free(buf);
    return 0;
}

int main(int argc, char **argv){
    if(argc!=2){
        fprintf(stderr,"Использование: %s <file>\n", argv[0]);
        return 1;
    }
    int fd = open(argv[1], O_RDONLY);
    if(fd<0){ perror("open"); return 1; }

    lines_t ls; build_index(fd, &ls);
    print_table(&ls);

    while(1){
        printf("Введите номер строки (1..%zu, 0 — выход): ", ls.size);
        fflush(stdout);
        char line[256];
        if(!fgets(line, sizeof(line), stdin)){
            printf("\nEOF. Завершение.\n");
            break;
        }
        char *end=NULL; errno=0;
        unsigned long long num = strtoull(line, &end, 10);
        if(errno==ERANGE){ fprintf(stderr,"Диапазон.\n"); continue; }
        while(end && (*end==' '||*end=='\t'||*end=='\n'||*end=='\r')) end++;
        if(end && *end!='\0'){ fprintf(stderr,"Некорректный ввод.\n"); continue; }
        if(num==0ULL) break;
        if(num<1ULL || num>(unsigned long long)ls.size){
            fprintf(stderr,"Нет такой строки. 1..%zu\n", ls.size); continue;
        }
        if(print_line(fd, &ls, (size_t)num)!=0){
            fprintf(stderr,"Ошибка печати строки %llu\n", num);
        }
    }

    lines_free(&ls);
    close(fd);
    return 0;
}
