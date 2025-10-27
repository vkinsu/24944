#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(void)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        execlp("cat", "cat", "./text.txt", NULL);
        printf("Bla-bla-bla");
        perror("exec cat");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Родитель: сейчас работает cat, а я продолжаю своё выполнение\n");
        printf("Родитель: могу делать что-то ещё, пока потомок выводит файл\n");
        printf("Родитель завершает работу\n");
    }

    return 0;
}
