#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

volatile bool timeout = false;

void timeout_handler(int sig) { timeout = true; }

int main()
{
    int st = open("input.txt", O_RDONLY);
    struct stat file_info;
    fstat(st, &file_info);

    int file_size = file_info.st_size;
    char* text = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, st, 0);

    int count_str = 0;
    for (int i = 0; i < file_size; i++) { if (text[i] == '\n') { count_str++;} }

    int pos_str[++count_str];
    pos_str[0] = 0;
    int idx = 1;

    for (int i = 0; i < file_size; i++)
    {
        if (text[i] == '\n') 
        {
            pos_str[idx] = i + 1;
            idx++;
        }
    }

    pos_str[count_str] = file_size;

    signal(SIGALRM, timeout_handler);
    alarm(5);

    printf("Введите номер строки (1-%d): ", count_str);

    int n;
    int result = scanf("%d", &n);
    
    if (result == 1)
    {
        alarm(0);
        for (int i = pos_str[n-1]; i < pos_str[n]; i++) { printf("%c", text[i]); }
    }
    else
    {
        for (int i = 0; i < file_size; i++) { printf("%c", text[i]); }
    }

    munmap(text, file_info.st_size);
    close(st);

    return 0;
}