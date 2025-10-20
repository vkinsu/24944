#include <unistd.h>
#include <stdio.h>
#include <errno.h>

void printUIDs(const char* stage) {
    printf("%s:\n", stage);
    printf("  Real UID: %d, Effective UID: %d\n", getuid(), geteuid());
}

void tryToOpen(char *filename) {
    printf("Trying to open: %s\n", filename);
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("  fopen failed");
        return;
    }

    printf("  File opened successfully!\n");
    fclose(file);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    printUIDs("BEFORE setuid()");
    tryToOpen(argv[1]);

    if (setuid(getuid()) == -1) {
        perror("setuid failed");
        return 1;
    }

    printUIDs("AFTER setuid()");
    tryToOpen(argv[1]);

    return 0;
}