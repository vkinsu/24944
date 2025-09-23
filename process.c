#define _XOPEN_SOURCE 700
#define _DARWIN_C_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>   // strcasecmp
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>

extern char **environ;

#ifndef RLIM_INFINITY
#define RLIM_INFINITY (~(rlim_t)0)
#endif

// Опции: -i -s -p -u -U <bytes|unlimited> -c -C <bytes|unlimited> -d -v -V name=value
// Обработка опций выполняется в обратном порядке (справа налево).

typedef struct {
    int   opt;
    char *arg;
    int   badopt;
} OptEntry;

// Печатает значение ресурсного лимита (например, RLIMIT_FSIZE или RLIMIT_CORE)
static void print_rlimit(const char *title, int resource) {
    struct rlimit rl;
    if (getrlimit(resource, &rl) != 0) { perror("getrlimit"); return; }
    printf("%s: ", title);
    if (rl.rlim_cur == RLIM_INFINITY) printf("unlimited");
    else printf("%llu", (unsigned long long)rl.rlim_cur);
    printf(" bytes\n");
}

// Устанавливает новый лимит ресурса (байты или "unlimited")
static int set_rlimit_value(int resource, const char *valstr, const char *flagname) {
    if (!valstr) { fprintf(stderr, "Опция -%s требует аргумент (байты или 'unlimited')\n", flagname); return -1; }

    rlim_t value;
    if (strcasecmp(valstr, "unlimited") == 0) {
        value = RLIM_INFINITY;
    } else {
        errno = 0;
        char *end = NULL;
        unsigned long long tmp = strtoull(valstr, &end, 10);
        if (errno || end == valstr || *end) {
            fprintf(stderr, "Недопустимое значение для -%s: '%s'\n", flagname, valstr);
            return -1;
        }
        value = (rlim_t)tmp;
    }

    struct rlimit rl;
    if (getrlimit(resource, &rl) != 0) { perror("getrlimit"); return -1; }
    rl.rlim_cur = value;
    if (value > rl.rlim_max) rl.rlim_max = value;   // может требовать привилегий
    if (setrlimit(resource, &rl) != 0) { perror("setrlimit"); return -1; }
    return 0;
}

// Выводит реальные и эффективные UID и GID
static void print_ids(void) {
    printf("UID(real)=%u  UID(eff)=%u  GID(real)=%u  GID(eff)=%u\n",
           (unsigned)getuid(), (unsigned)geteuid(), (unsigned)getgid(), (unsigned)getegid());
}

// Делает процесс лидером группы процессов (setpgid(0,0))
static void become_pgrp_leader(void) {
    if (setpgid(0, 0) != 0) perror("setpgid(0,0)");
    else printf("Процесс стал лидером группы (PGID=PID)\n");
}

// Выводит PID, PPID и PGRP процесса
static void print_pids(void) {
    printf("PID=%ld  PPID=%ld  PGRP=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp());
}

// Выводит лимит максимального размера создаваемого файла (ulimit -f)
static void print_ulimit_filesize(void) { print_rlimit("ulimit (RLIMIT_FSIZE)", RLIMIT_FSIZE); }

// Выводит лимит размера core-файла (ulimit -c)
static void print_core_limit(void)      { print_rlimit("core size (RLIMIT_CORE)", RLIMIT_CORE); }

// Выводит текущую рабочую директорию
static void print_cwd(void) {
    char *buf = getcwd(NULL, 0); if (!buf) { perror("getcwd"); return; }
    printf("cwd: %s\n", buf); free(buf);
}

// Устанавливает или изменяет переменную окружения (name=value)
static int set_env_name_value(const char *nv) {
    if (!nv) { fprintf(stderr, "Опция -V требует аргумент в форме name=value\n"); return -1; }
    const char *eq = strchr(nv, '=');
    if (!eq || eq == nv || *(eq + 1) == '\0') {
        fprintf(stderr, "Неверный формат для -V: '%s'\n", nv);
        return -1;
    }
    size_t name_len = (size_t)(eq - nv);
    char *name = (char*)malloc(name_len + 1);
    if (!name) { perror("malloc"); return -1; }
    memcpy(name, nv, name_len); name[name_len] = '\0';
    const char *value = eq + 1;
    if (setenv(name, value, 1) != 0) { perror("setenv"); free(name); return -1; }
    printf("ENV set: %s=%s\n", name, value);
    free(name);
    return 0;
}

// Печатает все переменные окружения
static void print_environ(void) {
    for (char **e = environ; e && *e; ++e) puts(*e);
}

// Точка входа: разбирает опции, сохраняет их и выполняет справа налево
int main(int argc, char *argv[]) {
    const char *optstr = "ispuU:cC:dvV:";
    int c;
    OptEntry *ops = NULL;
    size_t ops_cap = 0, ops_len = 0;

    opterr = 0;

    while ((c = getopt(argc, argv, optstr)) != -1) {
        if (ops_len == ops_cap) {
            size_t newcap = ops_cap ? ops_cap * 2 : 16;
            OptEntry *tmp = (OptEntry*)realloc(ops, newcap * sizeof(OptEntry));
            if (!tmp) { perror("realloc"); free(ops); return 1; }
            ops = tmp; ops_cap = newcap;
        }
        ops[ops_len].opt = c;
        ops[ops_len].arg = (optarg ? strdup(optarg) : NULL);
        ops[ops_len].badopt = (c == '?' ? optopt : 0);
        ops_len++;

        if (c == '?') {
            if (optopt) fprintf(stderr, "Недопустимая опция: -%c\n", optopt);
            else fprintf(stderr, "Ошибка разбора опций (возможно, отсутствует аргумент)\n");
        }
    }

    // Выполнение опций в обратном порядке
    for (ssize_t i = (ssize_t)ops_len - 1; i >= 0; --i) {
        int opt = ops[i].opt;
        char *arg = ops[i].arg;
        switch (opt) {
            case 'i': print_ids(); break;
            case 's': become_pgrp_leader(); break;
            case 'p': print_pids(); break;
            case 'u': print_ulimit_filesize(); break;
            case 'U': if (set_rlimit_value(RLIMIT_FSIZE, arg, "U") == 0) print_ulimit_filesize(); break;
            case 'c': print_core_limit(); break;
            case 'C': if (set_rlimit_value(RLIMIT_CORE,  arg, "C") == 0) print_core_limit(); break;
            case 'd': print_cwd(); break;
            case 'v': print_environ(); break;
            case 'V': set_env_name_value(arg); break;
            case '?': break;
            default:  break;
        }
    }

    // Если остались позиционные аргументы — вывести первый
    if (optind < argc) printf("Следующий позиционный параметр: %s\n", argv[optind]);

    for (size_t i = 0; i < ops_len; ++i) free(ops[i].arg);
    free(ops);
    return 0;
}
