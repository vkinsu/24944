#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define MAXL 500
#define BUFS 256

static volatile sig_atomic_t timed_out = 0;
static void on_alarm(int sig){ (void)sig; timed_out = 1; }

static void print_file_and_exit(int fd){
    char b[BUFS];
    lseek(fd, 0, SEEK_SET);
    for(;;){
        ssize_t r = read(fd, b, sizeof b);
        if (r <= 0) break;
        ssize_t w = 0;
        while (w < r){
            ssize_t k = write(1, b + w, r - w);
            if (k < 0) break;
            w += k;
        }
    }
    write(1, "\n", 1);
    _exit(0);
}

int main(int argc, char **argv){
    if (argc < 2){ fprintf(stderr, "usage: %s file\n", argv[0]); return 2; }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0){ perror("open"); return 1; }

    off_t off[MAXL+1]; int len[MAXL+1]; char nl[MAXL+1];
    int i=1, cur=0; char ch; off_t pos=0;
    off[1]=0;

    while (read(fd,&ch,1)==1){
        pos++;
        if (ch=='\n'){
            len[i]=cur+1; nl[i]=1; cur=0; off[++i]=pos;
            if(i>MAXL) break;
        } else cur++;
    }
    if (cur && i<=MAXL){ len[i]=cur; nl[i]=0; i++; }
    int cnt=i-1;
    if (cnt<=0){ puts("empty file"); close(fd); return 0; }

    struct sigaction sa = {0};
    sa.sa_handler = on_alarm;
    sigaction(SIGALRM,&sa,NULL);

    int first_prompt = 1;
    int n; char buf[BUFS];

    for(;;){
        timed_out = 0;
        printf("Line number (<=0 to exit): ");
        fflush(stdout);

        if (first_prompt) alarm(5);
        int rc = scanf("%d",&n);
        if (first_prompt) alarm(0);
        if (first_prompt && timed_out) print_file_and_exit(fd);
        first_prompt = 0;

        if (rc == EOF) break;                 
        if (rc != 1){
            int c2;
            while ((c2 = getchar()) != '\n' && c2 != EOF) {}
            fprintf(stderr, "Bad Line Number\n");
            continue;
        }

        if (n <= 0) break;
        if (n > cnt){ fprintf(stderr,"Bad Line Number\n"); continue; }

        if (lseek(fd, off[n], SEEK_SET) < 0){ perror("lseek"); break; }
        int left = len[n];
        while (left > 0){
            int chunk = left > BUFS ? BUFS : left;
            ssize_t r = read(fd, buf, chunk);
            if (r <= 0){ perror("read"); break; }
            ssize_t w=0;
            while (w<r){
                ssize_t k = write(1, buf+w, r-w);
                if (k<0){ perror("write"); break; }
                w+=k;
            }
            left -= (int)r;
        }
        if (!nl[n]) write(1, "\n", 1);
    }

    close(fd);
    return 0;
}
