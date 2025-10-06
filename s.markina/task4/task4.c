#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Структура для узла списка
struct Node {
    char* data;
    struct Node* next;
};

int main() {
    struct Node* head = NULL;
    struct Node* tail = NULL;  // Хвост списка для быстрого добавления
    char buffer[1024];         // Буфер для ввода строк

    printf("Вводите строки (для завершения введите '.' в начале строки):\n");

    while (1) {
        // Читаем строку в буфер и проверяем есть ли точка в начале строки
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {break;}
        if (buffer[0] == '.') {break;}

        // Определяем длину строки (без учета символа новой строки)
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') { // Убираем символ новой строки, если он есть
            buffer[len - 1] = '\0';
            len--;
        }

        // Выделяем память под строку
        char* str = (char*)malloc(len + 1);  // +1 для нулевого символа
        if (str == NULL) {
            fprintf(stderr, "Ошибка выделения памяти\n");
            exit(1);
        }

        // Копируем строку из буфера в выделенную память
        strcpy(str, buffer);

        // Создаем новый узел списка
        struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
        new_node->data = str;
        new_node->next = NULL;

        // Добавляем узел в конец списка
        if (head == NULL) {
            head = new_node;
            tail = new_node;
        }
        else {
            tail->next = new_node;
            tail = new_node;
        }
    }

    // Выводим все строки из списка
    printf("Введенные строки:\n");
    struct Node* current = head;
    while (current != NULL) {
        printf("%s\n", current->data);
        current = current->next;
    }

    // Освобождаем память
    current = head;
    while (current != NULL) {
        struct Node* next = current->next;
        free(current->data);  
        free(current);
        current = next;
    }

    return 0;
}