#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 Печатает реальный и эффективный идентификаторы пользователя
 Реальный UID - пользователь, который запустил программу
 Эффективный UID - пользователь, от имени которого выполняется программа
*/
static void print_user_ids(void)
{
    printf("Real user ID: %d\n", (int)getuid());
    printf("Effective user ID: %d\n", (int)geteuid());
}

/*
 Пытается открыть файл и возвращает результат операции
 filename - имя файла для открытия
 Возвращает 0 при успехе, -1 при ошибке
*/
static int try_open_file(const char *filename)
{
    printf("Attempting to open file: %s\n", filename);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen failed");
        return -1;
    }

    printf("File opened successfully\n");
    fclose(file);
    return 0;
}

int main(void)
{
    const char *filename = "secure_file.txt";

    /* STEP 1: начальный вывод UID */
    printf("=== STEP 1: Initial state ===\n");
    print_user_ids();

    /* STEP 2: пробуем открыть файл с текущими привилегиями */
    printf("\n=== STEP 2: Trying to open file ===\n");
    int result1 = try_open_file(filename);

    /* STEP 3: сбрасываем эффективный UID к реальному (удаляем повышенные привилегии) */
    printf("\n=== STEP 3: Setting effective UID to real UID ===\n");
    uid_t real_uid = getuid();

    if (setuid(real_uid) == -1) {
        perror("setuid failed");
        /* Всё равно выходим с кодом ошибки — сохраняем поведение */
        exit(EXIT_FAILURE);
    }

    printf("After setuid():\n");
    print_user_ids();

    /* STEP 4: пробуем открыть файл снова */
    printf("\n=== STEP 4: Trying to open file again ===\n");
    int result2 = try_open_file(filename);

    /* SUMMARY */
    printf("\n=== SUMMARY ===\n");
    printf("First attempt: %s\n", result1 == 0 ? "SUCCESS" : "FAILED");
    printf("Second attempt: %s\n", result2 == 0 ? "SUCCESS" : "FAILED");

    return 0;
}
