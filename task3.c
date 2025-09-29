/*
 * prog_setuid.c
 *
 * Программа демонстрирует работу с реальным и эффективным UID,
 * пытается открыть файл через fopen() до и после вызова setuid(geteuid()).
 *
 * Компиляция:
 *   gcc -Wall -Wextra -o prog_setuid prog_setuid.c
 *
 * Использование:
 *   ./prog_setuid путь/к/файлу
 *
 * Перед запуском: создайте файл и установите права 600
 *   touch datafile
 *   chmod 600 datafile
 *
 * Для проверки работы с SUID:
 *   chown root:yourgroup prog_setuid    # опционально, если хотите SUID от root
 *   chmod u+s prog_setuid
 *
 * (Комментарии оставлены для удобства — при необходимости можно удалить)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

static void print_uids(const char *label) {
    uid_t r = getuid();
    uid_t e = geteuid();
#ifdef __linux__
    /* Если доступна функция getresuid, можно показать и сохранённый UID */
    uid_t sr = (uid_t)-1, se = (uid_t)-1, ss = (uid_t)-1;
    if (getresuid(&sr, &se, &ss) == 0) {
        printf("%s: real=%u, effective=%u, saved=%u\n", label, (unsigned)sr, (unsigned)se, (unsigned)ss);
        return;
    }
#endif
    printf("%s: real=%u, effective=%u\n", label, (unsigned)r, (unsigned)e);
}

static void try_fopen(const char *path) {
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        perror("fopen");
        printf("Не удалось открыть '%s'\n", path);
    } else {
        printf("Файл '%s' успешно открыт (r). Закрываю файл.\n", path);
        if (fclose(f) != 0) {
            perror("fclose");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s путь/к/файлу\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *path = argv[1];

    /* Первичный вывод UID'ов и попытка открыть файл */
    print_uids("До setuid");
    try_fopen(path);

    /* Сделать так, чтобы реальный и эффективный UID совпадали.
     * Обычно вызывают setuid(geteuid()).
     */
    uid_t target = geteuid();
    if (setuid(target) == -1) {
        perror("setuid");
        printf("setuid(%u) завершился неудачей. Продолжаю для демонстрации.\n", (unsigned)target);
    } else {
        printf("setuid(%u) выполнен успешно.\n", (unsigned)target);
    }

    /* Повторный вывод UID'ов и повторная попытка открыть файл */
    print_uids("После setuid");
    try_fopen(path);

    return EXIT_SUCCESS;
}
