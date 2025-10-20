#define _XOPEN_SOURCE 700
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <ulimit.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define GET_FSLIM 1
#define SET_FSLIM 2

extern char **environ;

static void print_rlim(const char *label, rlim_t v) {
    if (v == RLIM_INFINITY)
        printf("%s = unlimited\n", label);
    else
        printf("%s = %llu\n", label, (unsigned long long)v);
}

static void print_max_user_processes(void) {
#ifdef RLIMIT_NPROC
    struct rlimit rlp;
    if (getrlimit(RLIMIT_NPROC, &rlp) == -1) {
        perror("getrlimit(RLIMIT_NPROC)");
    } else {
        print_rlim("max user processes (ulimit -u)", rlp.rlim_cur);
    }
#else
    /* Фолбэк для систем без RLIMIT_NPROC */
    long v = sysconf(_SC_CHILD_MAX);
    if (v == -1) {
        perror("sysconf(_SC_CHILD_MAX)");
    } else {
        /* _SC_CHILD_MAX возвращает число процессов на пользователя */
        printf("max user processes (ulimit -u) = %ld\n", v);
    }
#endif
}

int main(int argc, char *argv[]) {
    int c;
    const char options[] = "ispuU:cC:dvV:";
    struct rlimit rlp;
    char **p;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s options\n", argv[0]);
        exit(0);
    }

    while ((c = getopt(argc, argv, options)) != -1) {
        switch (c) {
        case 'i':
            printf("real userid = %u\n", getuid());
            printf("effective userid = %u\n", geteuid());
            printf("real groupid = %u\n", getgid());
            printf("effective groupid = %u\n", getegid());
            break;

        case 's':
            (void)setpgid(0, 0);
            break;

        case 'p':
            printf("process number = %d\n", getpid());
            printf("parent process number = %d\n", getppid());
            printf("group process number = %d\n", getpgid(0));
            break;

        case 'U':
            if (ulimit(SET_FSLIM, atol(optarg)) == -1)
                fprintf(stderr, "Must be super-user to increase ulimit\n");
            break;

        case 'u':
            print_max_user_processes();
            break;

        case 'c':
            if (getrlimit(RLIMIT_CORE, &rlp) == -1) {
                perror("getrlimit(RLIMIT_CORE)");
            } else {
                print_rlim("core size", rlp.rlim_cur);
            }
            break;

        case 'C':
            if (getrlimit(RLIMIT_CORE, &rlp) == -1) {
                perror("getrlimit(RLIMIT_CORE)");
            } else {
                rlp.rlim_cur = (rlim_t)atol(optarg);
                if (setrlimit(RLIMIT_CORE, &rlp) == -1)
                    fprintf(stderr, "Must be super-user to increase core\n");
            }
            break;

        case 'd': {
            char *cwd = getcwd(NULL, 0);
            if (!cwd) {
                perror("getcwd");
            } else {
                printf("current working directory is: %s\n", cwd);
                free(cwd);
            }
            break;
        }

        case 'v':
            printf("environment variables are:\n");
            for (p = environ; *p; p++)
                printf("%s\n", *p);
            break;

        case 'V':
            if (putenv(optarg) != 0)
                perror("putenv");
            break;

        default:
            fprintf(stderr, "Unknown option: -%c\n", c);
            break;
        }
    }

    return 0;
}
