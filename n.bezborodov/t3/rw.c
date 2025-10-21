/* suid_demo.c: печать RUID/EUID, попытка открыть файл, затем drop-privs и повтор */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

static void print_ids(void) {
    printf("RUID=%u  EUID=%u\n", (unsigned)getuid(), (unsigned)geteuid());
}

static void try_open(const char *path) {
    FILE *fp = fopen(path, "r+");          /* требуем и чтение, и запись */
    if (fp) {
        printf("OPEN OK: %s (r+)\n", path);
        fclose(fp);
    } else {
        perror("OPEN FAIL");
    }
}

int main(int argc, char **argv) {
    const char *path = (argc > 1) ? argv[1] : "data.txt";

    puts("--- Step 1 (before setuid) ---");
    print_ids();
    try_open(path);

    puts("--- Step 2 (setuid(getuid())) ---");
    if (setuid(getuid()) == -1) {
        perror("setuid");
        /* продолжаем, чтобы увидеть текущие ID и поведение */
    }
    print_ids();
    try_open(path);

    return 0;
}
