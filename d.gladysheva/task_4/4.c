#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node
{
    char *str;          // указатель на строку
    struct Node *next;  // указатель на следующий узел
} Node;

// фильтрация escape
void filter_escape_sequences(char* str)
{
    int i = 0, j = 0;
    int in_escape = 0;
    int escape_start = -1;
    
    while (str[i] != '\0')
    {
        if (in_escape)
        {
	    // Обработка случая ^[O - удаляем следующий символ
            if (str[i] == 'O' && i > 0 && str[i-1] == 27)
            {
                // Пропускаем текущий 'O' и следующий символ
                in_escape = 0;
                i++; // пропускаем 'O'
                if (str[i] != '\0') {
                    i++; // пропускаем следующий символ после 'O'
                }
                continue;
            }
            else if ((str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= 'a' && str[i] <= 'z') || str[i] == '~')
            {
                in_escape = 0;
            }
        }
        else if (str[i] == 27)
        { // ESC символ (^[)
            in_escape = 1;
            escape_start = i;
        }
        else if (((str[i] < 128) || (str[i] >= 192 && str[i] <= 253)))
        {
            if (str[i] == '[' && i > 0 && str[i-1] == 27) {}
            else { str[j++] = str[i]; }
        }
        else if ((str[i] >= 128 && str[i] <= 191)) { str[j++] = str[i]; }
        else if (str[i] >= 32 && str[i] <= 126) { str[j++] = str[i]; }
        
        i++;
    }
    str[j] = '\0';
}

int main()
{
    char buffer[1024];
    struct Node *head = NULL;
    struct Node *tail = NULL;

    while (fgets(buffer, sizeof(buffer), stdin) != NULL)
    {
        if (buffer[0] == '.') { break; }

        filter_escape_sequences(buffer);

        //замена \n на детерминант
        char *newline = strchr(buffer, '\n');
        if (newline != NULL) { *newline = '\0'; }

        int len_str = strlen(buffer);

        //копия поданой строки
        char *new_str = (char *)malloc(len_str + 1);
        strcpy(new_str, buffer);

        Node* list = (Node*)malloc(sizeof(Node));
        list->str = new_str;
        list->next = NULL;
        
        if (head == NULL) 
        {
            head = list;       // первый элемент
            tail = list;       // он же последний
        }
        else
        {
            tail->next = list; // текущий хвост ссылается на новый элемент
            tail = list;       // новый элемент становится хвостом
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
        free(to_free->str);
        free(to_free);
        current = current->next;
    }

    return 0;
}
