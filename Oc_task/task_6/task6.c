/* ===========================================================
 * task6.c — Таблица строк + печать таблицы + таймаут 5с на ПЕРВЫЙ ввод
 * Сборка: gcc -std=c11 -Wall -Wextra -O2 task6.c -o task6
 * =========================================================== */
#define _XOPEN_SOURCE 700
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
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
            }else line_len++;
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
    for(size_t i=0;i<ls->size;++i)
        printf("%-6zu %-12lld %-8zu\n", i+1, (long long)ls->data[i].offset, ls->data[i].len);
}

static void print_whole_and_exit(int fd){
    if(lseek(fd, 0L, SEEK_SET)==(off_t)-1){ perror("lseek"); }
    char buf[1<<15];
    ssize_t r;
    while((r=read(fd, buf, sizeof(buf)))>0){
        ssize_t off=0;
        while(off<r){
            ssize_t w = write(STDOUT_FILENO, buf+off, r-off);
            if(w<0){ if(errno==EINTR) continue; perror("write"); break; }
            off += w;
        }
    }
    if(r<0) perror("read");
    _exit(0);
}

static volatile sig_atomic_t g_timed_out=0;
static void on_alarm(int sig){ (void)sig; g_timed_out=1; }

static int read_number_with_optional_timeout(unsigned long long *out, unsigned timeout_sec){
    g_timed_out=0;
    if(timeout_sec>0) alarm(timeout_sec);
    char line[256];
    if(!fgets(line, sizeof(line), stdin)){
        if(timeout_sec>0 && g_timed_out) return 0;
        return 0;
    }
    if(timeout_sec>0) alarm(0);
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
    if(argc!=2){ fprintf(stderr,"Использование: %s <file>\n", argv[0]); return 1; }
    int fd = open(argv[1], O_RDONLY);
    if(fd<0){ perror("open"); return 1; }

    struct sigaction sa={0};
    sa.sa_handler=on_alarm;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGALRM,&sa,NULL)<0){ perror("sigaction"); close(fd); return 1; }

    lines_t ls; build_index(fd, &ls);
    print_table(&ls);

    int first_prompt=1;
    while(1){
        printf("Введите номер строки (1..%zu, 0 — выход)%s: ",
               ls.size, first_prompt?" [5 секунд]":"");
        fflush(stdout);

        unsigned long long num=0;
        int rc=read_number_with_optional_timeout(&num, first_prompt?5u:0u);

        if(first_prompt && rc==0){
            printf("\nТаймаут/EOF. Печатаю весь файл и выхожу:\n");
            print_whole_and_exit(fd);
        }
        if(first_prompt) first_prompt=0;

        if(rc<0){ fprintf(stderr,"Некорректный ввод.\n"); continue; }
        if(rc==0){ printf("\nEOF. Завершение.\n"); break; }
        if(num==0ULL) break;
        if(num<1ULL || num>(unsigned long long)ls.size){
            fprintf(stderr,"Нет такой строки.\n"); continue;
        }

        line_rec_t rec=ls.data[num-1];
        if(lseek(fd, rec.offset, SEEK_SET)==(off_t)-1){ perror("lseek"); continue; }
        char *buf=malloc(rec.len);
        if(!buf){ perror("malloc"); continue; }
        read(fd, buf, rec.len);
        write(STDOUT_FILENO, buf, rec.len);
        write(STDOUT_FILENO, "\n", 1);
        free(buf);
    }

    lines_free(&ls);
    close(fd);
    return 0;
}
