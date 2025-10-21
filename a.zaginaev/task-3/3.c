#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

static void print_user_ids(void) {
    printf("Real UID: %ld\n",  (long)getuid());
    printf("Effective UID: %ld\n", (long)geteuid());
}

static int try_open_file(const char *name) {
    printf("Attempting to open: %s\n", name);
    FILE *f = fopen(name, "r");
    if (!f) {
        perror("fopen");
        return -1;
    }
    puts("File opened successfully");
    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    const char *filename = (argc > 1) ? argv[1] : "secure_file.txt";

    puts("STEP 1: Initial state");
    print_user_ids();

    puts("\nSTEP 2: Try open with current privileges");
    int first = try_open_file(filename);

    puts("\nSTEP 3: Drop privileges (set effective UID = real UID)");
    uid_t ruid = getuid();
    if (setuid(ruid) == -1) {
        perror("setuid");
        return 1;
    }
    puts("After setuid():");
    print_user_ids();

    puts("\nSTEP 4: Try open again after dropping privileges");
    int second = try_open_file(filename);

    puts("\n=== SUMMARY ===");
    printf("First attempt:  %s\n", (first  == 0) ? "SUCCESS" : "FAILED");
    printf("Second attempt: %s\n", (second == 0) ? "SUCCESS" : "FAILED");

    return 0;
}
