#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

int file;
long long *offsets;
int *lengths;
int line_count;

void timeout_handler(int sig)
{
    (void)sig;
    printf("\nВремя истекло!\n\n");
    char buf[1024];
    if (lseek(file, 0, SEEK_SET) == -1)
        perror("lseek");
    ssize_t r;
    while ((r = read(file, buf, sizeof(buf))) > 0)
        write(STDOUT_FILENO, buf, r);
    printf("\n");
    free(offsets);
    free(lengths);
    close(file);
    exit(0);
}

int main()
{
    const char *filename = "input.txt";
    file = open(filename, O_RDONLY);
    if (file < 0)
    {
        perror("open");
        return 1;
    }

    offsets = malloc(sizeof(long long));
    lengths = malloc(sizeof(int));
    if (!offsets || !lengths)
    {
        perror("malloc");
        close(file);
        return 1;
    }

    line_count = 0;
    offsets[line_count++] = 0;

    long long pos = 0;
    char c;
    long long line_start = 0;
    while (read(file, &c, 1) == 1)
    {
        pos++;
        if (c == '\n')
        {
            lengths[line_count - 1] = (int)(pos - line_start - 1);
            offsets = realloc(offsets, (line_count + 1) * sizeof(long long));
            lengths = realloc(lengths, (line_count + 1) * sizeof(int));
            if (!offsets || !lengths)
            {
                perror("realloc");
                close(file);
                return 1;
            }
            offsets[line_count++] = pos;
            line_start = pos;
        }
    }
    if (line_start < pos)
        lengths[line_count - 1] = (int)(pos - line_start);

    printf("Таблица смещений и длин строк:\n");
    for (int i = 0; i < line_count; i++)
        printf("Строка %d: смещение = %lld, длина = %d\n", i + 1, offsets[i], lengths[i]);

    char *line_buf = NULL;
    int number;

    signal(SIGALRM, timeout_handler);

    alarm(5);
    if (scanf("%d", &number) != 1)
    {
        printf("\nНекорректный ввод\n");
        free(offsets);
        free(lengths);
        close(file);
        return 1;
    }

    alarm(0);

    while (1)
    {
        if (number == 0)
            break;
        if (number < 1 || number > line_count)
        {
            printf("Неверный номер строки\n");
        }
        else
        {
            int len = lengths[number - 1];
            line_buf = realloc(line_buf, len + 1);
            if (!line_buf)
            {
                perror("realloc");
                break;
            }

            if (lseek(file, offsets[number - 1], SEEK_SET) == -1)
            {
                perror("lseek");
                break;
            }

            if (read(file, line_buf, len) != len)
            {
                perror("read");
                break;
            }

            line_buf[len] = '\0';
            printf("Строка %d: %s", number, line_buf);
            if (line_buf[len - 1] != '\n')
                printf("\n");
        }

        printf("\nВведите номер строки начиная с 1 или 0 для выхода: ");
        fflush(stdout);

        if (scanf("%d", &number) != 1)
        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            continue;
        }
    }

    free(offsets);
    free(lengths);
    free(line_buf);
    close(file);
    return 0;
}
