#include <unistd.h>
#include <stdio.h>
#include <errno.h>

void UID_show(const char* st) {
    printf("%s:\n", st);
    printf("REAL UID: %d| EFFECTIVE UID: %d\n", getuid(), geteuid());
}

void Open(char *file_a) {
    printf("Trying to open: %s\n", file_a);
    FILE *file = fopen(file_a, "r");

    if (file == NULL) {
        perror(" FOPEN FAILED");
        return;
    }

    fclose(file);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("USAGE: %s <filename>\n", argv[0]);
        return 1;
    }

    printUIDs("BEFORE setuid()");
    Open(argv[1]);

    if (setuid(getuid()) == -1) {
        perror("SETUP FAILED");
        return 1;
    }

    printUIDs();
    Open(argv[1]);

    return 0;
}