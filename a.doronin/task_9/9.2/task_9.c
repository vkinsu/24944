#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid = fork();
    printf("Родитель начал работать.");
    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        execlp("cat", "cat", "./text.txt", NULL);
        perror("exec cat");
        exit(EXIT_FAILURE);
    }
    else
    {
        int status;
        pid_t w = waitpid(pid, &status, 0); // макрос возвращает -1 если неудача иначе PID

        if (w == -1)
        {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status)) // если процесс завершился корректно через exit или return
        {
            printf("Потомок завершился с кодом %d\n", WEXITSTATUS(status)); // вовзращает то что передано в exit или return
        }
        else if (WIFSIGNALED(status)) // если процесс завершился сигналом sigkill sigsegv
        {
            printf("Родитель: потомок убит сигналом %d\n", WTERMSIG(status)); // номер сигнала убившего потомка
        }
        printf("Родитель завершает работу\n");
    }

    return 0;
}
