#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct ListNode {
    char *line;
    struct ListNode *next;
};

static void chomp(char *s) {
    size_t n = strlen(s);
    if (n && (s[n-1] == '\n' || s[n-1] == '\r')) s[--n] = '\0';
    if (n && s[n-1] == '\r') s[--n] = '\0';
}

static int is_blank(const char *s) {
    for (; *s; ++s) if (!isspace((unsigned char)*s)) return 0;
    return 1;
}

/* Удаляем ANSI-escape, включая стрелки: ESC [ A/B/C/D и прочие CSI-последовательности */
static void scrub_ansi(char *s) {
    size_t r = 0, w = 0;
    while (s[r]) {
        unsigned char c = (unsigned char)s[r];
        if (c == 0x1B) {                /* ESC */
            r++;
            if (s[r] == '[') {          /* CSI */
                r++;
                /* пропустить параметры/промежуточные до финала 0x40..0x7E */
                while (s[r]) {
                    unsigned char t = (unsigned char)s[r++];
                    if (t >= 0x40 && t <= 0x7E) break;
                }
            } else if (s[r]) {
                /* краткая ESC-последовательность: пропустить один символ */
                r++;
            }
            continue;
        }
        if (c >= 32 && c != 127) {      /* печатаемые (кроме DEL) */
            s[w++] = s[r++];
        } else {
            r++;                         /* управляющие — игнор */
        }
    }
    s[w] = '\0';
}

static struct ListNode* node_new(const char *line) {
    struct ListNode *n = (struct ListNode*)malloc(sizeof *n);
    if (!n) { perror("malloc node"); exit(1); }
    size_t len = strlen(line);
    n->line = (char*)malloc(len + 1);
    if (!n->line) { perror("malloc line"); exit(1); }
    memcpy(n->line, line, len + 1);
    n->next = NULL;
    return n;
}

int main(void) {
    char buf[BUFSIZ];
    struct ListNode head = { NULL, NULL };  /* фиктивный узел на стеке */
    struct ListNode *tail = &head;

    while (fgets(buf, sizeof buf, stdin)) {
        if (buf[0] == '.') break;           /* точка в начале — конец ввода */
        chomp(buf);                          /* убрать \n/\r */
        scrub_ansi(buf);                     /* убрать ESC/стрелки */
        if (!buf[0] || is_blank(buf)) continue;

        tail->next = node_new(buf);
        tail = tail->next;
    }

    for (struct ListNode *p = head.next; p; p = p->next)
        puts(p->line);

    /* освобождение памяти */
    struct ListNode *p = head.next;
    while (p) {
        struct ListNode *n = p->next;
        free(p->line);
        free(p);
        p = n;
    }
    return 0;
}
