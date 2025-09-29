#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

volatile bool timeout = false;

void timeout_handler(int sig) { timeout = true; }

int main()
{
    int st = open("input.txt", O_RDONLY);

    char buffer[1024];
    int read_bites = read(st, buffer, sizeof(buffer));

    int count_str = 0;
    for (int i = 0; i < read_bites; i++) { if (buffer[i] == '\n') { count_str++;} }

    int pos_str[++count_str];
    pos_str[0] = 0;
    int idx = 1;

    for (int i = 0; i < read_bites; i++)
    {
        if (buffer[i] == '\n') 
        {
            pos_str[idx] = i + 1;
            idx++;
        }
    }

    pos_str[count_str] = read_bites;

    signal(SIGALRM, timeout_handler);

    while(1)
    {
        alarm(5);

        printf("Введите номер строки (1-%d): ", count_str);

        int n;
        int result = scanf("%d", &n);
        
        if (n == 0) { break; }

        if (result == 1)
        {
            alarm(0);

            lseek(st, pos_str[n-1], SEEK_SET);

            int index = pos_str[n] - pos_str[n-1];
            char output[index];
            read(st, output, index);

            for (int i = 0; i < index; i++) { printf("%c", output[i]); }
        }
        else
        {
            for (int i = 0; i < read_bites; i++) { printf("%c", buffer[i]); }
        }
    }

    close(st);

    return 0;
}
