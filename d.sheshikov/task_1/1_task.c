#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

extern char **environ;

typedef struct {
    char type;
    char *param;
} Operation;

int main(int arg_count, char *arg_values[]) {
    char option_string[] = "ispuU:cC:dvV:";
    Operation *op_list = NULL;
    int op_count = 0;
    int current_option;

    printf("Number of arguments: %d\n", arg_count);

    while ((current_option = getopt(arg_count, arg_values, option_string)) != -1) {
        if (current_option == '?') {
            printf("Unknown option: %c\n", optopt);
            continue;
        }

        Operation *new_list = realloc(op_list, (op_count + 1) * sizeof(Operation));
        if (!new_list) {
            fprintf(stderr, "Memory error\n");
            return 1;
        }
        op_list = new_list;
        op_list[op_count].type = current_option;
        op_list[op_count].param = optarg ? strdup(optarg) : NULL;
        op_count++;
    }

    for (int idx = op_count - 1; idx >= 0; idx--) {
        char opt_type = op_list[idx].type;
        char *opt_param = op_list[idx].param;
        
        switch (opt_type) {
            case 'i':
                printf("User IDs - Real: %d, Effective: %d, Group Real: %d, Group Effective: %d\n",
                       getuid(), geteuid(), getgid(), getegid());
                break;
            
            case 's':
                if (setpgid(0, 0) != 0) {
                    fprintf(stderr, "Group creation failed\n");
                }
                break;
            
            case 'p':
                printf("Process Info - ID: %d, Parent: %d, Group: %d\n",
                       getpid(), getppid(), getpgrp());
                break;
            
            case 'u': {
                struct rlimit limits;
                if (getrlimit(RLIMIT_NPROC, &limits) == 0) {
                    if (limits.rlim_cur == RLIM_INFINITY) {
                        printf("Max processes: unlimited\n");
                    } else {
                        printf("Max processes: %lld\n", (long long)limits.rlim_cur);
                    }
                } else {
                    perror("getrlimit error for RLIMIT_NPROC");
                }
                break;
            }
            
            case 'U': {
                if (opt_param) {
                    if (strcmp(opt_param, "unlimited") == 0) {
                        struct rlimit rlim;
                        if (getrlimit(RLIMIT_NPROC, &rlim) == 0) {
                            rlim.rlim_cur = RLIM_INFINITY;
                            if (setrlimit(RLIMIT_NPROC, &rlim) < 0) {
                                perror("setrlimit error for RLIMIT_NPROC");
                            }
                        }
                    } else {
                        char *end_ptr;
                        errno = 0;
                        long new_val = strtol(opt_param, &end_ptr, 10);
                        
                        if (end_ptr == opt_param || *end_ptr != '\0' || errno != 0 || new_val < 0) {
                            fprintf(stderr, "Invalid process limit value: %s\n", opt_param);
                        } else {
                            struct rlimit rlim;
                            if (getrlimit(RLIMIT_NPROC, &rlim) == 0) {
                                rlim.rlim_cur = new_val;
                                if (setrlimit(RLIMIT_NPROC, &rlim) < 0) {
                                    perror("setrlimit error for RLIMIT_NPROC");
                                }
                            } else {
                                perror("getrlimit error");
                            }
                        }
                    }
                }
                break;
            }
            
            case 'c': {
                struct rlimit rlim;
                if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
                    if (rlim.rlim_cur == RLIM_INFINITY) {
                        printf("Core limit: unlimited\n");
                    } else {
                        printf("Core limit: %lld\n", (long long)rlim.rlim_cur);
                    }
                }
                break;
            }
            
            case 'C': {
                if (opt_param) {
                    char *end_ptr;
                    long core_val = strtol(opt_param, &end_ptr, 10);
                    if (*end_ptr == '\0' && core_val >= 0) {
                        struct rlimit rlim = {core_val, core_val};
                        setrlimit(RLIMIT_CORE, &rlim);
                    } else {
                        fprintf(stderr, "Bad core size: %s\n", opt_param);
                    }
                }
                break;
            }
            
            case 'd': {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd))) {
                    printf("Directory: %s\n", cwd);
                }
                break;
            }
            
            case 'v':
                for (char **env = environ; *env; env++) {
                    printf("%s\n", *env);
                }
                break;
            
            case 'V':
                if (opt_param && strchr(opt_param, '=')) {
                    putenv(opt_param);
                } else {
                    fprintf(stderr, "Env format error: %s\n", opt_param);
                }
                break;
        }
    }

    for (int idx = 0; idx < op_count; idx++) {
        if (op_list[idx].param) {
            free(op_list[idx].param);
        }
    }
    free(op_list);

    printf("Processed options: %d\n", op_count);
    printf("optind: %d\n", optind);
    if (optind < arg_count) {
        printf("Remaining: %s\n", arg_values[optind]);
    }

    return 0;
}