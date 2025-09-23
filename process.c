#define _XOPEN_SOURCE 700
#define _DARWIN_C_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>

extern char **environ;

#ifndef RLIM_INFINITY
#define RLIM_INFINITY (~(rlim_t)0)
#endif

typedef struct {
    int opt;        // символ опции, например 'i', 'p', 'U', 'V', '?' и т.д.
    char *arg;      // аргумент опции (если есть), иначе NULL
    int badopt;     // если opt=='?', сюда положим неверный символ опции (optopt)
} OptEntry;

static void print_rlimit(const char *title, int resource) {
    struct rlimit rl;
    if (getrlimit(resource, &rl) != 0) {
        perror("getrlimit");
        return;
    }
    printf("%s: ", title);
    if (rl.rlim_cur == RLIM_INFINITY) {
        printf("unlimited");
    } else {
        printf("%llu", (unsigned long long)rl.rlim_cur);
    }
    printf(" bytes\n");
}

static int set_rlimit_value(int resource, const char *valstr, const char *flagname) {
    errno = 0;
    char *end = NULL;
    // Допускаем слово "unlimited" (без регистра) как RLIM_INFINITY
    rlim_t value;
    if (valstr == NULL) {
        fprintf(stderr, "Опция -%s требует аргумент (байты или 'unlimited')\n", flagname);
        return -1;
    }
    if (strcasecmp(valstr, "unlimited") == 0) {
        value = RLIM_INFINITY;
    } else {
        unsigned long long tmp = strtoull(valstr, &end, 10);
        if (errno != 0 || end == valstr || *end != '\0') {
            fprintf(stderr, "Недопустимое значение для -%s: '%s'\n", flagname, valstr ? valstr : "(null)");
            return -1;
        }
        value = (rlim_t)tmp;
    }

    struct rlimit rl;
    if (getrlimit(resource, &rl) != 0) {
        perror("getrlimit");
        return -1;
    }
    rl.rlim_cur = value;
    // Обычно нельзя превышать rlim_max без привилегий; попытаемся также поднять max, если это допустимо
    if (value > rl.rlim_max) rl.rlim_max = value;

    if (setrlimit(resource, &rl) != 0) {
        perror("setrlimit");
        return -1;
    }
    return 0;
}

static void print_ids(void) {
    uid_t ruid = getuid();
    uid_t euid = geteuid();
    gid_t rgid = getgid();
    gid_t egid = getegid();
    printf("UID(real)=%u  UID(eff)=%u  GID(real)=%u  GID(eff)=%u\n",
           (unsigned)ruid, (unsigned)euid, (unsigned)rgid, (unsigned)egid);
}

static void become_pgrp_leader(void) {
    // setpgid(0,0) -> установить PGID равным PID, сделать процесс лидером своей группы
    if (setpgid(0, 0) != 0) {
        perror("setpgid(0,0)");
    } else {
        printf("Процесс стал лидером группы (PGID=PID)\n");
    }
}

static void print_pids(void) {
    pid_t pid  = getpid();
    pid_t ppid = getppid();
    pid_t pgrp = getpgrp();
    printf("PID=%ld  PPID=%ld  PGRP=%ld\n", (long)pid, (long)ppid, (long)pgrp);
}

static void print_ulimit_filesize(void) {
    print_rlimit("ulimit (RLIMIT_FSIZE)", RLIMIT_FSIZE);
}

static void print_core_limit(void) {
    print_rlimit("core size (RLIMIT_CORE)", RLIMIT_CORE);
}

static void print_cwd(void) {
    char *buf = NULL;
    size_t sz = 0;
    // POSIX: getcwd(NULL, 0) — реаллокация внутри
    buf = getcwd(NULL, 0);
    if (!buf) {
        perror("getcwd");
        return;
    }
    printf("cwd: %s\n", buf);
    free(buf);
    (void)sz;
}

static int set_env_name_value(const char *nv) {
    if (!nv) {
        fprintf(stderr, "Опция -V требует аргумент в форме name=value\n");
        return -1;
    }
    const char *eq = strchr(nv, '=');
    if (!eq || eq == nv || *(eq + 1) == '\0') {
        fprintf(stderr, "Неверный формат для -V: ожидается name=value, получено '%s'\n", nv);
        return -1;
    }
    size_t name_len = (size_t)(eq - nv);
    char *name = (char*)malloc(name_len + 1);
    if (!name) {
        perror("malloc");
        return -1;
    }
    memcpy(name, nv, name_len);
    name[name_len] = '\0';
    const char *value = eq + 1;

    if (setenv(name, value, 1) != 0) {
        perror("setenv");
        free(name);
        return -1;
    }
    printf("ENV set: %s=%s\n", name, value);
    free(name);
    return 0;
}

static void print_environ(void) {
    for (char **e = environ; e && *e; ++e) {
        puts(*e);
    }
}

int main(int argc, char *argv[]) {
    // Строка допустимых опций: опции с аргументом помечены ':'
    // i s p u U: c C: d v V:
    const char *optstr = "ispuU:cC:dvV:";
    int c;

    // Собираем опции (в порядке слева направо), чтобы затем исполнить их справа налево.
    OptEntry *ops = NULL;
    size_t ops_cap = 0, ops_len = 0;

    // Отключим автоматическое сообщение об ошибках от getopt
    opterr = 0;

    // Сбрасываем глобальные переменные getopt на случай повторных запусков через exec (не обязательно, но аккуратно)
    // optind по умолчанию = 1.

    while ((c = getopt(argc, argv, optstr)) != -1) {
        if (ops_len == ops_cap) {
            size_t newcap = ops_cap ? ops_cap * 2 : 16;
            OptEntry *tmp = (OptEntry*)realloc(ops, newcap * sizeof(OptEntry));
            if (!tmp) {
                perror("realloc");
                free(ops);
                return 1;
            }
            ops = tmp;
            ops_cap = newcap;
        }
        ops[ops_len].opt = c;
        ops[ops_len].arg = (optarg ? strdup(optarg) : NULL);
        ops[ops_len].badopt = (c == '?' ? optopt : 0);
        ops_len++;

        // Если опция недопустима, сами выведем диагностическое сообщение (как в примере).
        if (c == '?') {
            if (optopt) {
                fprintf(stderr, "Недопустимая опция: -%c\n", optopt);
            } else {
                // Случай, когда getopt возвращает '?' из-за отсутствия аргумента у опции с ':'
                // В этом случае optopt содержит опцию, у которой отсутствует аргумент (POSIX).
                fprintf(stderr, "Ошибка разбора опций (возможно, отсутствует аргумент у опции)\n");
            }
        }
    }

    // Теперь исполняем опции в обратном порядке (справа налево).
    for (ssize_t i = (ssize_t)ops_len - 1; i >= 0; --i) {
        int opt = ops[i].opt;
        char *arg = ops[i].arg;
        switch (opt) {
            case 'i':
                print_ids();
                break;
            case 's':
                become_pgrp_leader();
                break;
            case 'p':
                print_pids();
                break;
            case 'u':
                print_ulimit_filesize();
                break;
            case 'U':
                if (set_rlimit_value(RLIMIT_FSIZE, arg, "U") == 0) {
                    // После установки — покажем текущее значение
                    print_ulimit_filesize();
                }
                break;
            case 'c':
                print_core_limit();
                break;
            case 'C':
                if (set_rlimit_value(RLIMIT_CORE, arg, "C") == 0) {
                    print_core_limit();
                }
                break;
            case 'd':
                print_cwd();
                break;
            case 'v':
                print_environ();
                break;
            case 'V':
                set_env_name_value(arg);
                break;
            case '?':
                // Уже напечатали диагностическое сообщение выше.
                // Здесь можно дополнительно учесть порядок, но смысла мало.
                break;
            default:
                // Неожиданное — не должно случаться.
                break;
        }
    }

    // После обработки опций справа налево, если остались позиционные аргументы — выведем первый (как в примере)
    if (optind < argc) {
        printf("Следующий позиционный параметр: %s\n", argv[optind]);
    } else {
        // Нет позиционных аргументов — можно ничего не печатать.
        // printf("Позиционных аргументов нет.\n");
    }

    // Освобождение ресурсов
    for (size_t i = 0; i < ops_len; ++i) {
        free(ops[i].arg);
    }
    free(ops);

    return 0;
}
