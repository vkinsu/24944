#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Структура для узла списка
struct Node {
    char* data;
    struct Node* next;
};

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

int main() {
    struct Node* head = NULL;
    struct Node* tail = NULL;
    char buffer[1024];

    printf("Вводите строки (для завершения введите '.' в начале строки):\n");

    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        // Проверяем на точку ДО фильтрации
        if (buffer[0] == '.') {
            break;
        }

        // ФИЛЬТРАЦИЯ ESCAPE-ПОСЛЕДОВАТЕЛЬНОСТЕЙ
        filter_escape_sequences(buffer);

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        // Пропускаем пустые строки после фильтрации
        if (len == 0) {continue;}
        char* str = (char*)malloc(len + 1);
        strcpy(str, buffer);

        struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
        
        new_node->data = str;
        new_node->next = NULL;

        if (head == NULL) {
            head = new_node;
            tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }
    }

    printf("Введенные строки:\n");
    struct Node* current = head;
    while (current != NULL) {
        printf("%s\n", current->data);
        current = current->next;
    }

    current = head;
    while (current != NULL) {
        struct Node* next = current->next;
        free(current->data);  
        free(current);
        current = next;
    }

    return 0;
}