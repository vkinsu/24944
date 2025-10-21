/* ===========================================================
 * task7.c — Таблица строк (mmap) + печать таблицы + таймаут 5с на первый ввод
 * Сборка: gcc -std=c11 -Wall -Wextra -O2 task7.c -o task7
 * =========================================================== */
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
    if(ls->size==ls->cap){
        size_t ncap=ls->cap?ls->cap*2:256;
        line_rec_t *nd=realloc(ls->data,ncap*sizeof(*nd));
        if(!nd){ perror("realloc"); exit(1); }
        ls->data=nd; ls->cap=ncap;
    }
    ls->data[ls->size].offset=off;
    ls->data[ls->size].len=len;
    ls->size++;
}

static void print_table(const lines_t *ls){
    printf("Таблица строк (всего %zu):\n", ls->size);
    printf("%-6s %-12s %-8s\n", "№", "offset", "len");
    for(size_t i=0;i<ls->size;++i)
        printf("%-6zu %-12lld %-8zu\n", i+1, (long long)ls->data[i].offset, ls->data[i].len);
}

static volatile sig_atomic_t g_timed_out=0;
static void on_alarm(int sig){ (void)sig; g_timed_out=1; }

static int read_number_with_optional_timeout(unsigned long long *out, unsigned timeout_sec){
    g_timed_out=0;
    if(timeout_sec>0) alarm(timeout_sec);
    char line[256];
    if(!fgets(line,sizeof(line),stdin)){
        if(timeout_sec>0 && g_timed_out) return 0;
        return 0;
    }
    if(timeout_sec>0) alarm(0);
    char *end=NULL;
    errno=0;
    unsigned long long val=strtoull(line,&end,10);
    if(errno==ERANGE) return -1;
    while(end && (*end==' '||*end=='\t'||*end=='\n'||*end=='\r')) end++;
    if(end && *end!='\0') return -1;
    *out=val;
    return 1;
}

static void write_all(const char *p,size_t len){
    size_t left=len;
    while(left>0){
        ssize_t w=write(STDOUT_FILENO,p,left>SSIZE_MAX?SSIZE_MAX:(ssize_t)left);
        if(w<0){ if(errno==EINTR) continue; perror("write"); break; }
        left-=w; p+=w;
    }
}

int main(int argc,char **argv){
    if(argc!=2){ fprintf(stderr,"Использование: %s <file>\n",argv[0]); return 1; }

    int fd=open(argv[1],O_RDONLY);
    if(fd<0){ perror("open"); return 1; }

    struct stat st;
    if(fstat(fd,&st)<0){ perror("fstat"); close(fd); return 1; }
    off_t fsz=st.st_size;
    if(fsz==0){ puts("Файл пуст."); close(fd); return 0; }

    const char *base=mmap(NULL,fsz,PROT_READ,MAP_PRIVATE,fd,0);
    if(base==(void*)-1){ perror("mmap"); close(fd); return 1; }

    lines_t ls; lines_init(&ls);
    off_t start=0; size_t len=0;
    for(off_t i=0;i<fsz;i++){
        if(base[i]=='\n'){ lines_push(&ls, start, len); start=i+1; len=0; } else { len++; }
    }
    if(start < fsz){ lines_push(&ls, start, (size_t)(fsz-start)); }

    print_table(&ls);

    struct sigaction sa={0};
    sa.sa_handler=on_alarm;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGALRM,&sa,NULL)<0){
        perror("sigaction");
        lines_free(&ls);
        munmap((void*)base,fsz);
        close(fd);
        return 1;
    }

    int first_prompt=1;
    while(1){
        printf("Введите номер строки (1..%zu, 0 — выход)%s: ",
               ls.size, first_prompt?" [5 секунд]":"");
        fflush(stdout);

        unsigned long long num=0;
        int rc=read_number_with_optional_timeout(&num, first_prompt?5u:0u);

        if(first_prompt && rc==0){
            printf("\nТаймаут/EOF. Печатаю весь файл и выхожу:\n");
            write_all(base,(size_t)fsz);
            lines_free(&ls);
            munmap((void*)base,fsz);
            close(fd);
            return 0;
        }
        if(first_prompt) first_prompt=0;

        if(rc<0){ fprintf(stderr,"Некорректный ввод.\n"); continue; }
        if(rc==0){ printf("\nEOF. Завершение.\n"); break; }
        if(num==0ULL) break;
        if(num<1ULL || num>(unsigned long long)ls.size){
            fprintf(stderr,"Нет такой строки.\n"); continue;
        }

        line_rec_t rec=ls.data[num-1];
        write_all(base+rec.offset, rec.len);
        if(write(STDOUT_FILENO,"\n",1)<0) perror("write");
    }

    lines_free(&ls);
    munmap((void*)base,fsz);
    close(fd);
    return 0;
}
