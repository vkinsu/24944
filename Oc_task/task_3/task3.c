#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static void print_uids(void) {
    printf("Real UID: %d\n", (int)getuid());
    printf("Effective UID: %d\n", (int)geteuid());
}

static int try_open(const char *filename) {
    printf("Attempt to open '%s' for reading...\n", filename);
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        return -1;
    }
    printf("fopen succeeded\n");
    fclose(f);
    return 0;
}

int main(int argc, char *argv[]) {
    const char *filename = "secure_file.txt";
    if (argc >= 2) filename = argv[1];

    printf("=== STEP 1: initial ===\n");
    print_uids();

    printf("\n=== STEP 2: open with current IDs ===\n");
    int r1 = try_open(filename);

    printf("\n=== STEP 3: setuid(getuid()) - make effective UID == real UID ===\n");
    uid_t ruid = getuid();
    if (setuid(ruid) == -1) {
        /* setuid может вернуть -1 и установить errno */
        fprintf(stderr, "setuid(%d) failed: %s\n", (int)ruid, strerror(errno));
        /* В большинстве случаев стоит выйти с ошибкой, т.к. мы не добились требуемого поведения */
        exit(EXIT_FAILURE);
    }

    printf("After setuid():\n");
    print_uids();

    printf("\n=== STEP 4: open after setuid ===\n");
    int r2 = try_open(filename);

    printf("\n=== SUMMARY ===\n");
    printf("First attempt:  %s\n", r1 == 0 ? "SUCCESS" : "FAILED");
    printf("Second attempt: %s\n", r2 == 0 ? "SUCCESS" : "FAILED");

    return (r1 == 0 || r2 == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
