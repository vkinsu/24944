#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//подредачить текст и проверить для сдачи

typedef struct Node
{
    char *str;
    struct Node *next;
} Node;

int main()
{
    char buffer[1024];
    struct Node *head = NULL;
    struct Node *tail = NULL;

    while (fgets(buffer, sizeof(buffer), stdin) != NULL)
    {
        if (buffer[0] == '.') { break; }

        char *newline = strchr(buffer, '\n');
        if (newline != NULL) { *newline = '\0'; }

        int len_str = strlen(buffer);

        char *new_str = (char *)malloc(len_str + 1);
        strcpy(new_str, buffer);

        Node* list = (Node*)malloc(sizeof(struct Node));

        list->str = new_str;
        list->next = NULL;
        
        if (head == NULL) 
        {
            head = list;
            tail = list;
        }
        else
        {
            tail->next = list;
            tail = list;
        }
    }

    struct Node *current = head;
    while (current != NULL)
    {
        printf("%s\n", current->str);
        current = current->next;
    }
    
    current = head;
    while (current != NULL)
    {
        struct Node *to_free = current;
        current = current->next;
        free(to_free->str);
        free(to_free);
    }

    return 0;
}