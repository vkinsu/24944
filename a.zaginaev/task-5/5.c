#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXL 500
#define BUFS 256

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s file\n", argv[0]); return 2; }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    off_t off[MAXL + 1];      
    int   len[MAXL + 1];     
    char  nl[MAXL + 1];      
    int   i = 1, cur = 0;     
    char  ch;              
    off_t pos = 0;            

    off[1] = 0;              
    while (read(fd, &ch, 1) == 1) {
        pos++;
        if (ch == '\n') {
            len[i] = cur + 1; nl[i] = 1; cur = 0;
            off[++i] = pos;  
            if (i > MAXL) break;
        } else cur++;
    }
    if (cur && i <= MAXL) { len[i] = cur; nl[i] = 0; i++; } 
    int cnt = i - 1;
    if (cnt <= 0) { puts("empty file"); close(fd); return 0; }

    int n;
    char buf[BUFS];
    for (;;) {
        printf("Line number (<=0 to exit): "); fflush(stdout);
        if (scanf("%d", &n) != 1 || n <= 0) break;
        if (n > cnt) { fprintf(stderr, "Bad Line Number\n"); continue; }

        if (lseek(fd, off[n], SEEK_SET) < 0) { perror("lseek"); break; }
        int left = len[n];
        while (left > 0) {
            int chunk = left > BUFS ? BUFS : left;
            int r = read(fd, buf, chunk);
            if (r <= 0) { perror("read"); break; }
            int w = 0; while (w < r) { int k = write(1, buf + w, r - w); if (k < 0) { perror("write"); break; } w += k; }
            left -= r;
        }
        if (!nl[n]) write(1, "\n", 1);
    }

    close(fd);
    return 0;
}
