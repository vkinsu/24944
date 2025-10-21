#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUFFER_SIZE 1024

typedef struct list_element {
    char *text;
    struct list_element *next_element;
} list_element;

int main() {
    list_element *first = NULL, *last = NULL;
    char user_input[INPUT_BUFFER_SIZE];
    
    printf("Type your text lines (enter a line starting with dot to stop):\n");
    
    while (fgets(user_input, INPUT_BUFFER_SIZE, stdin) != NULL) {
        if (user_input[0] == '.') {
            break;
        }
        
        size_t input_length = strlen(user_input);
        if (input_length > 0 && user_input[input_length - 1] == '\n') {
            user_input[input_length - 1] = '\0';
            input_length--;
        }
        
        char *text_storage = malloc(input_length + 1);
        if (text_storage == NULL) {
            perror("Failed to allocate memory");
            exit(1);
        }
        strcpy(text_storage, user_input);
        
        list_element *new_item = malloc(sizeof(list_element));
        if (new_item == NULL) {
            perror("Failed to allocate memory");
            free(text_storage);
            exit(1);
        }
        new_item->text = text_storage;
        new_item->next_element = NULL;
        
        if (first == NULL) {
            first = new_item;
            last = new_item;
        } else {
            last->next_element = new_item;
            last = new_item;
        }
    }
    
    printf("\nYour entered lines:\n");
    list_element *current_item = first;
    while (current_item != NULL) {
        printf("%s\n", current_item->text);
        current_item = current_item->next_element;
    }
    
    current_item = first;
    while (current_item != NULL) {
        list_element *following_item = current_item->next_element;
        free(current_item->text);
        free(current_item);
        current_item = following_item;
    }
    
    return 0;
}