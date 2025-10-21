#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 4096

typedef struct Node {
    char *s;
    struct Node *next;
} Node;

static void free_list(Node *head) {
    while (head) {
        Node *n = head->next;
        free(head->s);
        free(head);
        head = n;
    }
}

int main(void) {
    Node *head = NULL, *tail = NULL;
    char buf[MAX_LINE];

    while (fgets(buf, sizeof(buf), stdin)) {
        if (buf[0] == '.') break;

        size_t len = strlen(buf);
        if (len && buf[len - 1] == '\n') buf[--len] = '\0';

        char *copy = (char *)malloc(len + 1);
        if (!copy) { perror("malloc"); free_list(head); return 1; }
        memcpy(copy, buf, len + 1);

        Node *node = (Node *)malloc(sizeof(*node));
        if (!node) { perror("malloc"); free(copy); free_list(head); return 1; }
        node->s = copy;
        node->next = NULL;

        if (!head) head = tail = node;
        else { tail->next = node; tail = node; }
    }

    for (Node *p = head; p; p = p->next) puts(p->s);

    free_list(head);
    return 0;
}
