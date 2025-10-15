#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

void printUIDs(void) {
    printf("real UID: %d, effective UID: %d\n", getuid(), geteuid());
}

void tryToOpen(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open the file");
        return;
    }
    printf("Successfully opened \"%s\"\n", filename);
    fclose(file);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    printUIDs();
    tryToOpen(filename);

    if (setuid(getuid()) == -1) {
        perror("setuid failed");

    } else {
        printf("setuid(getuid()) succeeded — privileges (if any) dropped.\n");
    }

    printUIDs();
    tryToOpen(filename);

    return 0;
}
