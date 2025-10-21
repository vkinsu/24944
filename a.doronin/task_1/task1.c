#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

extern char **environ;

typedef struct
{
    int opt;
    char *arg;
} OptRec;

int main(int argc, char *argv[])
{
    const char *optstring = "ispuU:cC:dvV:";
    struct rlimit limit;
    int opt;

    OptRec *arr = NULL;
    size_t arr_len = 0;

    optind = 1;
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {

        OptRec rec;
        rec.opt = opt;
        if (optarg)
            rec.arg = strdup(optarg);
        else
            rec.arg = NULL;

        OptRec *tmp = realloc(arr, (arr_len + 1) * sizeof(OptRec));
        if (!tmp)
        {
            perror("realloc");
            for (size_t i = 0; i < arr_len; ++i)
                free(arr[i].arg);
            free(arr);
            return 1;
        }
        arr = tmp;
        arr[arr_len++] = rec;
    }

    for (ssize_t i = (ssize_t)arr_len - 1; i >= 0; --i)
    {
        int o = arr[i].opt;
        char *a = arr[i].arg;

        switch (o)
        {
        case 'i':
            printf("User IDs:\n");
            printf("  Real UID: %d\n", getuid());
            printf("  Effective UID: %d\n", geteuid());
            printf("Group IDs:\n");
            printf("  Real GID: %d\n", getgid());
            printf("  Effective GID: %d\n\n", getegid());
            break;

        case 's':
            if (setpgid(0, getpid()) == -1)
                perror("setpgid");
            else
                printf("Process %d is now group leader (PGID=%d)\n", getpid(), getpgid(0));
            break;

        case 'p':
            printf("Process information:\n");
            printf("  PID:  %d\n", getpid());
            printf("  PPID: %d\n", getppid());
            printf("  PGID: %d\n", getpgid(0));
            break;

        case 'u':
            if (getrlimit(RLIMIT_NPROC, &limit) == -1)
                perror("getrlimit (RLIMIT_NPROC)");
            else
                printf("Process limit (ulimit): Soft=%ld Hard=%ld\n",
                       (long)limit.rlim_cur, (long)limit.rlim_max);
            break;

        case 'U':
        {
            if (!a)
            {
                fprintf(stderr, "Option -U requires an argument\n");
                break;
            }
            errno = 0;
            char *endptr = NULL;
            long val = strtol(a, &endptr, 10);
            if (endptr == a || *endptr != '\0' || errno != 0 || val < 0)
            {
                fprintf(stderr, "Invalid value for -U: %s\n", a);
                break;
            }
            if (getrlimit(RLIMIT_NPROC, &limit) == -1)
            {
                perror("getrlimit (RLIMIT_NPROC)");
            }
            else
            {
                limit.rlim_cur = (rlim_t)val;
                if (setrlimit(RLIMIT_NPROC, &limit) == -1)
                    perror("setrlimit (RLIMIT_NPROC)");
                else
                    printf("Process limit changed to: %ld\n", val);
            }
            break;
        }

        case 'c':
            if (getrlimit(RLIMIT_CORE, &limit) == -1)
                perror("getrlimit (RLIMIT_CORE)");
            else
                printf("Core file size limits: Soft=%lu Hard=%lu\n",
                       (unsigned long)limit.rlim_cur,
                       (unsigned long)limit.rlim_max);
            break;

        case 'C':
        {
            if (!a)
            {
                fprintf(stderr, "Option -C requires an argument\n");
                break;
            }
            errno = 0;
            char *endptr = NULL;
            long val = strtol(a, &endptr, 10);
            if (endptr == a || *endptr != '\0' || errno != 0 || val < 0)
            {
                fprintf(stderr, "Invalid value for -C: %s\n", a);
                break;
            }
            if (getrlimit(RLIMIT_CORE, &limit) == -1)
            {
                perror("getrlimit (RLIMIT_CORE)");
            }
            else
            {
                limit.rlim_cur = (rlim_t)val;
                if (setrlimit(RLIMIT_CORE, &limit) == -1)
                    perror("setrlimit (RLIMIT_CORE)");
                else
                    printf("New core file limit set to %ld bytes\n", val);
            }
            break;
        }

        case 'd':
        {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) == NULL)
                perror("getcwd");
            else
                printf("Current working directory: %s\n", cwd);
            break;
        }

        case 'v':
            printf("Environment variables:\n");
            for (char **env = environ; *env != NULL; ++env)
                printf("  %s\n", *env);
            break;

        case 'V':
            if (!a)
            {
                fprintf(stderr, "Option -V requires an argument name=value\n");
                break;
            }
            if (putenv(a) != 0)
                perror("putenv");
            else
                printf("Environment variable updated: %s\n", a);
            arr[i].arg = NULL;
            break;

        case '?':
            fprintf(stderr, "Unknown option encountered\n");
            break;

        default:
            fprintf(stderr, "Unexpected option: -%c\n", o);
            break;
        }
    }

    for (size_t i = 0; i < arr_len; ++i)
        if (arr[i].arg)
            free(arr[i].arg);

    free(arr);
    return 0;
}
