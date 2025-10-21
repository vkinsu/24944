#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node
{
    char *str;
    struct Node *next;
};

int main(void)
{
    const int MAX_LEN = 256;
    char buffer[MAX_LEN];
    struct Node *head = NULL, *tail = NULL;

    printf("Введите строки \nЕсли хотит закончить введите точку \".\"");

    while (1)
    {
        if (!fgets(buffer, MAX_LEN, stdin))
            break;

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
            buffer[--len] = '\0';

        if (buffer[0] == '.')
            break;

        char *line = (char *)malloc((len + 1) * sizeof(char));
        if (!line)
        {
            fprintf(stderr, "Ошибка выделения памяти\n");
            return 1;
        }
        strcpy(line, buffer);

        struct Node *node = (struct Node *)malloc(sizeof(struct Node));
        if (!node)
        {
            fprintf(stderr, "Ошибка выделения памяти\n");
            free(line);
            return 1;
        }

        node->str = line;
        node->next = NULL;

        if (head == NULL)
            head = tail = node;
        else
        {
            tail->next = node;
            tail = node;
        }
    }

    printf("\nВведённые строки:\n");
    struct Node *cur = head;
    while (cur != NULL)
    {
        printf("%s\n", cur->str);
        cur = cur->next;
    }

    cur = head;
    while (cur != NULL)
    {
        struct Node *tmp = cur;
        cur = cur->next;
        free(tmp->str);
        free(tmp);
    }

    return 0;
}
