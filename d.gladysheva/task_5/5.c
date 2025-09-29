#include <stdio.h>
#include <fcntl.h>

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

    while(1)
    {
        int n;
        scanf("%d", &n);

        if (n == 0) { break; }

        lseek(st, pos_str[n-1], 0);

        int index = pos_str[n] - pos_str[n-1];
        char output[index];
        read(st, output, index);

        for (int i = 0; i < index; i++) { printf("%c", output[i]); }
    }

    close(st);

    return 0;
}
