#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
    char *data;
    struct node *next;
};

static struct node *create(const char *input);
static void free_list(struct node *head);

int main(void)
{
    char line[BUFSIZ];
    struct node *head = NULL, *here = NULL, *p = NULL;

    /* Инициализация связного списка с фиктивной головой */
    head = (struct node *)malloc(sizeof(struct node));
    if (!head) {
        perror("malloc");
        return 1;
    }
    head->data = NULL;
    head->next = NULL;
    here = head;

    /* Чтение строк и добавление в список */
    printf("Enter lines of text:\n");
    while (fgets(line, sizeof(line), stdin) != NULL) {
        /* останов по строке, начинающейся с точки */
        if (line[0] == '.')
            break;

        /* убрать завершающий '\n' если есть */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        here->next = create(line);
        if (!here->next) {
            fprintf(stderr, "Allocation failed\n");
            free_list(head);
            return 1;
        }
        here = here->next;
    }

    /* Печать списка */
    for (p = head->next; p != NULL; p = p->next)
        puts(p->data);

    /* Освобождение памяти */
    free_list(head);
    return 0;
}

static struct node *create(const char *input)
{
    struct node *q = (struct node *)malloc(sizeof(struct node));
    if (!q)
        return NULL;

    q->data = (char *)malloc(strlen(input) + 1);
    if (!q->data) {
        free(q);
        return NULL;
    }
    strcpy(q->data, input);
    q->next = NULL;
    return q;
}

static void free_list(struct node *head)
{
    struct node *p = head;
    while (p) {
        struct node *next = p->next;
        free(p->data);
        free(p);
        p = next;
    }
}
