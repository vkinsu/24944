#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

char *file_data = NULL;
long long *offsets;
int line_count;
size_t file_size = 0;
int *lengths;
int file;

void timeout(int sig)
{
    (void)sig;
    printf("\nВремя истекло!\n\n");
    write(STDOUT_FILENO, file_data, file_size);
    printf("\n");

    free(offsets);
    free(lengths);
    if (file_data)
        munmap(file_data, file_size);
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

    struct stat st;
    if (fstat(file, &st) == -1)
    {
        perror("fstat");
        close(file);
        return 1;
    }
    file_size = st.st_size;

    file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, file, 0);
    if (file_data == MAP_FAILED)
    {
        perror("mmap");
        close(file);
        return 1;
    }

    offsets = malloc(sizeof(long long));
    lengths = malloc(sizeof(int));
    if (!offsets || !lengths)
    {
        perror("malloc");
        munmap(file_data, file_size);
        close(file);
        return 1;
    }

    line_count = 0;
    offsets[line_count++] = 0;

    long long line_start = 0;
    for (long long pos = 0; pos < (long long)file_size; pos++)
    {
        if (file_data[pos] == '\n')
        {
            lengths[line_count - 1] = (int)(pos - line_start);

            offsets = realloc(offsets, (line_count + 1) * sizeof(long long));
            lengths = realloc(lengths, (line_count + 1) * sizeof(int));
            if (!offsets || !lengths)
            {
                perror("realloc");
                munmap(file_data, file_size);
                close(file);
                return 1;
            }

            offsets[line_count++] = pos + 1;
            line_start = pos + 1;
        }
    }

    if (line_start < (long long)file_size)
        lengths[line_count - 1] = (int)(file_size - line_start);

    printf("Таблица смещений и длин строк:\n");
    for (int i = 0; i < line_count; i++)
        printf("Строка %d: смещение = %lld, длина = %d\n", i + 1, offsets[i], lengths[i]);

    char *line_buf = NULL;
    int number;

    signal(SIGALRM, timeout);

    while (1)
    {
        printf("\nВведите номер строки начиная с 1 или 0 для выхода: ");
        fflush(stdout);

        alarm(5);

        if (scanf("%d", &number) != 1)
        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            continue;
        }

        alarm(0);

        if (number == 0)
            break;
        if (number < 1 || number > line_count)
        {
            printf("Неверный номер строки\n");
            continue;
        }

        int len = lengths[number - 1];
        line_buf = realloc(line_buf, len + 1);
        if (!line_buf)
        {
            perror("realloc");
            break;
        }

        memcpy(line_buf, &file_data[offsets[number - 1]], len);
        line_buf[len] = '\0';
        printf("Строка %d: %s", number, line_buf);
        if (line_buf[len - 1] != '\n')
            printf("\n");
    }

    free(offsets);
    free(lengths);
    free(line_buf);
    if (file_data)
        munmap(file_data, file_size);
    close(file);
    return 0;
}
