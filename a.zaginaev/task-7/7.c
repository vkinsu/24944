#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXL 500

static volatile sig_atomic_t timed_out = 0;
static void on_alarm(int s){ (void)s; timed_out = 1; }

int main(int argc, char **argv){
    if (argc < 2){ fprintf(stderr,"usage: %s file\n", argv[0]); return 2; }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0){ perror("open"); return 1; }

    struct stat st;
    if (fstat(fd, &st) == -1){ perror("fstat"); close(fd); return 1; }
    size_t sz = (size_t)st.st_size;

    if (sz == 0){ puts("empty file"); close(fd); return 0; }

    char *base = mmap(NULL, sz, PROT_READ, MAP_PRIVATE, fd, 0);
    if (base == MAP_FAILED){ perror("mmap"); close(fd); return 1; }

    size_t off[MAXL+1]; int len[MAXL+1]; char nl[MAXL+1];
    int i = 1, cur = 0;
    off[1] = 0;

    for (size_t p = 0; p < sz; ++p){
        if (base[p] == '\n'){
            len[i] = cur + 1; nl[i] = 1; cur = 0;
            if (++i > MAXL) break;
            off[i] = p + 1;
        } else cur++;
    }
    if (cur && i <= MAXL){ len[i] = cur; nl[i] = 0; i++; }

    int cnt = i - 1;
    if (cnt <= 0){ puts("empty file"); munmap(base, sz); close(fd); return 0; }

    struct sigaction sa = {0}; sa.sa_handler = on_alarm; sigaction(SIGALRM, &sa, NULL);

    int first = 1, n;
    while (1){
        timed_out = 0;
        printf("Line number (<=0 to exit): ");
        fflush(stdout);

        if (first) alarm(5);
        int rc = scanf("%d", &n);
        if (first) alarm(0);
        if (first && timed_out){
            (void)fwrite(base, 1, sz, stdout);
            if (sz==0 || base[sz-1] != '\n') fputc('\n', stdout);
            fflush(stdout);
            munmap(base, sz); close(fd);
            _exit(0);
        }
        first = 0;

        if (rc == EOF) break;
        if (rc != 1){
            int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
            fprintf(stderr, "Bad Line Number\n");
            continue;
        }
        if (n <= 0) break;
        if (n > cnt){ fprintf(stderr, "Bad Line Number\n"); continue; }

        (void)fwrite(base + off[n], 1, (size_t)len[n], stdout);
        if (!nl[n]) fputc('\n', stdout);
        fflush(stdout);
    }

    munmap(base, sz);
    close(fd);
    return 0;
}
