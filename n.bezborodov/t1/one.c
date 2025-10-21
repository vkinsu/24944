// proc_attr.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <sys/resource.h>
#include <limits.h>

extern char **environ;

static void print_ids(void) {
    uid_t ruid = getuid(), euid = geteuid();
    gid_t rgid = getgid(), egid = getegid();
    struct passwd *rpw = getpwuid(ruid), *epw = getpwuid(euid);
    struct group  *rg  = getgrgid(rgid),  *eg  = getgrgid(egid);

    printf("Real UID: %u (%s)\n",  ruid, rpw ? rpw->pw_name : "?");
    printf("Eff. UID: %u (%s)\n",  euid, epw ? epw->pw_name : "?");
    printf("Real GID: %u (%s)\n",  rgid, rg  ? rg->gr_name  : "?");
    printf("Eff. GID: %u (%s)\n",  egid, eg  ? eg->gr_name  : "?");
}

static void print_pids(void) {
    pid_t pid = getpid(), ppid = getppid(), pgid = getpgrp();
    printf("PID=%ld PPID=%ld PGID=%ld\n", (long)pid, (long)ppid, (long)pgid);
}

static void make_group_leader(void) {
    // стать лидером группы процессов
    if (setpgid(0, 0) == -1) {
        perror("setpgid");
    } else {
        printf("Made current process a process-group leader. PGID=%ld\n", (long)getpgrp());
    }
}

static void print_rlim(const char *what, int res) {
    struct rlimit rl;
    if (getrlimit(res, &rl) == -1) { perror("getrlimit"); return; }
    if (rl.rlim_cur == RLIM_INFINITY)
        printf("%s (soft): unlimited\n", what);
    else
        printf("%s (soft): %llu bytes\n", what, (unsigned long long)rl.rlim_cur);
}

static void set_rlim_bytes(const char *what, int res, unsigned long long val) {
    struct rlimit rl;
    if (getrlimit(res, &rl) == -1) { perror("getrlimit"); return; }
    rl.rlim_cur = (rlim_t)val;
    if (val > rl.rlim_max) rl.rlim_max = (rlim_t)val; // попытка поднять max, если можно
    if (setrlimit(res, &rl) == -1) {
        fprintf(stderr, "setrlimit(%s) failed: %s\n", what, strerror(errno));
    } else {
        print_rlim(what, res);
    }
}

static void print_cwd(void) {
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf)) == NULL) { perror("getcwd"); return; }
    printf("cwd: %s\n", buf);
}

static void print_env(void) {
    for (char **e = environ; *e; ++e) puts(*e);
}

int main(int argc, char *argv[]) {
    int opt;
    int invalid = 0;

    opterr = 0; // сами печатаем про неизвестные опции
    while ((opt = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        switch (opt) {
            case 'i': print_ids(); break;
            case 's': make_group_leader(); break;
            case 'p': print_pids(); break;

            case 'u': // «ulimit» — аналогично печатаем лимит размера файла
                print_rlim("ulimit(file size)", RLIMIT_FSIZE);
                break;
            case 'U': { // установить «ulimit»
                char *end = NULL;
                unsigned long long v = strtoull(optarg, &end, 10);
                if (!optarg[0] || (end && *end)) {
                    fprintf(stderr, "Bad value for -U: %s\n", optarg);
                    invalid++;
                } else {
                    set_rlim_bytes("ulimit(file size)", RLIMIT_FSIZE, v);
                }
                break;
            }

            case 'c': // размер core-файла
                print_rlim("core size", RLIMIT_CORE);
                break;
            case 'C': { // установить размер core-файла
                char *end = NULL;
                unsigned long long v = strtoull(optarg, &end, 10);
                if (!optarg[0] || (end && *end)) {
                    fprintf(stderr, "Bad value for -C: %s\n", optarg);
                    invalid++;
                } else {
                    set_rlim_bytes("core size", RLIMIT_CORE, v);
                }
                break;
            }

            case 'd': print_cwd(); break;

            case 'v': print_env(); break;

            case 'V': { // Vname=value
                if (strchr(optarg, '=') == NULL) {
                    fprintf(stderr, "-V requires name=value, got: %s\n", optarg);
                    invalid++;
                } else {
                    // putenv требует строку, живущую после возврата
                    char *s = strdup(optarg);
                    if (!s) { perror("strdup"); break; }
                    if (putenv(s) != 0) {
                        perror("putenv");
                        free(s); // в случае ошибки освобождаем
                    } else {
                        printf("set env: %s\n", optarg);
                    }
                }
                break;
            }

            case '?':
                if (optopt)
                    fprintf(stderr, "illegal option -- %c\n", optopt);
                else
                    fprintf(stderr, "illegal option\n");
                invalid++;
                break;
        }
    }

    if (invalid)
        fprintf(stderr, "invalid options count: %d\n", invalid);

    // показать оставшиеся позиционные параметры
    for (int i = optind; i < argc; ++i) {
        printf("arg[%d]=%s\n", i, argv[i]);
    }
    return invalid ? 1 : 0;
}
