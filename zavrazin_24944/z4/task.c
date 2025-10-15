#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

typedef struct Node {
    char *str;
    struct Node *next;
} Node;

void set_raw_mode(int enable) {
    static struct termios oldt, newt;

    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

int main() {
    char buffer[1024];
    int pos = 0;

    Node *head = NULL;
    Node *tail = NULL;

    printf("Введите строки ('.' на новой строке — выход):\n");

    set_raw_mode(1);

    while (1) {
        int c = getchar();

        if (c == 27) {
            getchar();
            getchar();
            continue;
        }

        if (c == '\n' || c == '\r') {
            buffer[pos] = '\0';

            if (buffer[0] == '.' && pos == 1)
                break;

            if (pos > 0) {
                char *new_str = malloc(pos + 1);
                strcpy(new_str, buffer);

                Node *node = malloc(sizeof(Node));
                node->str = new_str;
                node->next = NULL;

                if (head == NULL)
                    head = tail = node;
                else {
                    tail->next = node;
                    tail = node;
                }
            }

            pos = 0;
            printf("\n");
            continue;
        }

        if (isalpha(c) || c == ' ' || c == '.') {
            buffer[pos++] = c;
            putchar(c); 
            fflush(stdout);
        }
    }

    set_raw_mode(0);

    printf("\n\nРезультат:\n");
    Node *cur = head;
    while (cur) {
        printf("%s\n", cur->str);
        cur = cur->next;
    }

    cur = head;
    while (cur) {
        Node *tmp = cur;
        cur = cur->next;
        free(tmp->str);
        free(tmp);
    }

    return 0;
}
