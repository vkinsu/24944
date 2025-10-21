#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_LINES 1024
#define MAX_LINE_LEN 1024

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        return 1;
    }

    off_t offsets[MAX_LINES];
    int lengths[MAX_LINES];
    int line_count = 0;
    off_t pos = 0, line_start = 0;
    char ch;

    // Построение таблицы смещений и длин строк
    while (read(fd, &ch, 1) == 1) {
        pos++;
        if (ch == '\n') {
            offsets[line_count] = line_start;
            lengths[line_count] = pos - line_start;
            line_start = pos;
            line_count++;
            if (line_count >= MAX_LINES) break;
        }
    }

    // Добавляем последнюю строку, если файл не оканчивается \n
    if (pos > line_start && line_count < MAX_LINES) {
        offsets[line_count] = line_start;
        lengths[line_count] = pos - line_start;
        line_count++;
    }

    int n;
    char linebuf[MAX_LINE_LEN];

    while (1) {
        printf("Enter line number (0 to exit): ");
        if (scanf("%d", &n) != 1) break;
        if (n == 0) break;

        if (n < 1 || n > line_count) {
            printf("Invalid line number\n");
            continue;
        }

        off_t offset = offsets[n - 1];
        int len = lengths[n - 1];

        lseek(fd, offset, SEEK_SET);
        int bytes_read = read(fd, linebuf, len);
        if (bytes_read <= 0) {
            perror("read");
            continue;
        }

        linebuf[bytes_read] = '\0';
        printf("%s", linebuf);
        if (linebuf[bytes_read - 1] != '\n') printf("\n");
    }

    close(fd);
    return 0;
}
