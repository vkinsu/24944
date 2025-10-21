#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
    char *text;
    struct node *next;
};

static void strip_newline(char *s) {
    size_t n = strlen(s);
    if (n && s[n-1] == '\n') s[n-1] = '\0';
}

static void sanitize_line(char *s) {
    size_t i = 0, j = 0;
    while (s[i]) {
        unsigned char c = (unsigned char)s[i];
        if (c == 0x1B) {                 
            i++;
            if (s[i] == '[') {           
                i++;
                while (s[i]) {           
                    unsigned char x = (unsigned char)s[i++];
                    if (x >= 0x40 && x <= 0x7E) break;
                }
            } else if (s[i]) {
                i++;                    
            }
            continue;
        }
        if (c >= 32 && c != 127) {      
            s[j++] = s[i++];
        } else {
            i++;                        
        }
    }
    s[j] = '\0';
}

static struct node* make_node(const char *text) {
    struct node *n = malloc(sizeof *n);
    if (!n) { perror("malloc node"); exit(1); }
    size_t len = strlen(text);
    n->text = malloc(len + 2);           
    if (!n->text) { perror("malloc text"); exit(1); }
    memcpy(n->text, text, len);
    n->text[len] = '\n';
    n->text[len+1] = '\0';
    n->next = NULL;
    return n;
}

int main(void) {
    char buf[BUFSIZ];
    struct node *head = malloc(sizeof *head), *cur;
    if (!head) { perror("malloc head"); return 1; }
    head->text = NULL; head->next = NULL; cur = head;

    while (fgets(buf, sizeof buf, stdin)) {
        if (buf[0] == '.') break;      
        strip_newline(buf);               
        sanitize_line(buf);               
        if (buf[0] == '\0') continue;     
        cur->next = make_node(buf);
        cur = cur->next;
    }

    for (struct node *p = head->next; p; p = p->next)
        printf("%s", p->text);

    for (struct node *p = head, *n; p; p = n) {
        n = p->next; free(p->text); free(p);
    }
    return 0;
}
