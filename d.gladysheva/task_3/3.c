#include <stdio.h>
#include <errno.h>
#include <unistd.h>


void print_r_and_ef()
{
    printf("Real: %d\n", getuid());
    printf("Effective: %d\n", geteuid());
}

int main()
{
    print_r_and_ef();

    FILE* st = fopen("my_secret_file.txt", "r");
    if (st == NULL) { perror("Ошибка открытия файла"); }
    else { printf("Succes\n"); }
    fclose(st);

    if (setuid(getuid()) == -1) { perror("setuid failed"); }
    print_r_and_ef();

    FILE* fn = fopen("my_secret_file.txt", "r");
    if (fn == NULL) { perror("Ошибка открытия файла"); }
    else { printf("Succes\n"); }
    fclose(fn);

    return 0;
}