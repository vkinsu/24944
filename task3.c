#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Демонстрация реального и эффективного UID и открытия файла до и после setuid(getuid()).
 * Компиляция: gcc -o setuid_demo setuid_demo.c
 */

int main(int argc, char *argv[])
{
    FILE *fp;
    uid_t uid;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s file_name\n", argv[0]);
        exit(1);
    }

    /* Печатаем текущие реальные и эффективные UID */
    printf("Initially: real uid = %u, effective uid = %u\n",
           (unsigned)getuid(), (unsigned)geteuid());

    /* Первый попытка открыть файл (с текущими правами euid) */
    if ((fp = fopen(argv[1], "r")) == NULL) {
        perror("first open failed");
    } else {
        printf("first open successful\n");
        fclose(fp);
    }

    /* Сбрасываем эффективный UID к реальному (удаляем повышенные права) */
    uid = getuid();
    if (setuid(uid) == -1) {
        perror("setuid failed");
        /* продолжим, хотя обычно setuid должен пройти */
    }

    printf("After setuid(getuid()): real uid = %u, effective uid = %u\n",
           (unsigned)getuid(), (unsigned)geteuid());

    /* Вторая попытка открыть тот же файл уже без повышенных прав */
    if ((fp = fopen(argv[1], "r")) == NULL) {
        perror("second open failed");
    } else {
        printf("second open successful\n");
        fclose(fp);
    }

    return 0;
}
