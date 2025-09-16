#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <limits.h>

void print_user_ids() {
    printf("=== User and Group IDs ===\n");
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
    printf("Real GID: %d\n", getgid());
    printf("Effective GID: %d\n", getegid());
    printf("\n");
}

void become_group_leader() {
    printf("=== Becoming Group Leader ===\n");
    if (setpgid(0, 0) == -1) {
        perror("setpgid failed");
    } else {
        printf("Process became group leader successfully\n");
        printf("New Group ID: %d\n", getpgrp());
    }
    printf("\n");
}

void print_process_ids() {
    printf("=== Process IDs ===\n");
    printf("Process ID (PID): %d\n", getpid());
    printf("Parent Process ID (PPID): %d\n", getppid());
    printf("Process Group ID: %d\n", getpgrp());
    printf("\n");
}

void print_ulimit() {
    printf("=== ulimit ===\n");
    struct rlimit rlim;
    if (getrlimit(RLIMIT_FSIZE, &rlim) == 0) {
        printf("ulimit: %ld bytes\n", (long)rlim.rlim_cur);
    } else {
        perror("getrlimit failed");
    }
    printf("\n");
}

void change_ulimit(const char *value) {
    printf("=== Changing ulimit ===\n");
    long new_limit = atol(value);
    if (new_limit <= 0) {
        printf("Error: Invalid ulimit value '%s'\n", value);
        return;
    }
    
    struct rlimit rlim;
    if (getrlimit(RLIMIT_FSIZE, &rlim) == 0) {
        rlim.rlim_cur = new_limit;
        if (setrlimit(RLIMIT_FSIZE, &rlim) == -1) {
            perror("setrlimit failed");
        } else {
            printf("ulimit changed to: %ld bytes\n", new_limit);
        }
    } else {
        perror("getrlimit failed");
    }
    printf("\n");
}

void print_core_size() {
    printf("=== Core File Size ===\n");
    struct rlimit rlim;
    if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
        printf("Core file size: %ld bytes\n", (long)rlim.rlim_cur);
    } else {
        perror("getrlimit failed");
    }
    printf("\n");
}

void change_core_size(const char *value) {
    printf("=== Changing Core File Size ===\n");
    long new_size = atol(value);
    if (new_size < 0) {
        printf("Error: Invalid core size value '%s'\n", value);
        return;
    }
    
    struct rlimit rlim;
    if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
        rlim.rlim_cur = new_size;
        if (setrlimit(RLIMIT_CORE, &rlim) == -1) {
            perror("setrlimit failed");
        } else {
            printf("Core file size changed to: %ld bytes\n", new_size);
        }
    } else {
        perror("getrlimit failed");
    }
    printf("\n");
}

void print_current_directory() {
    printf("=== Current Directory ===\n");
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current directory: %s\n", cwd);
    } else {
        perror("getcwd failed");
    }
    printf("\n");
}

void print_environment() {
    printf("=== Environment Variables ===\n");
    extern char **environ;
    char **env = environ;
    
    while (*env) {
        printf("%s\n", *env);
        env++;
    }
    printf("\n");
}

void set_environment_variable(const char *name_value) {
    printf("=== Setting Environment Variable ===\n");
    
    // Создаем копию строки для модификации
    char *copy = strdup(name_value);
    if (copy == NULL) {
        perror("strdup failed");
        return;
    }
    
    char *equals = strchr(copy, '=');
    if (equals) {
        *equals = '\0';
        char *name = copy;
        char *value = equals + 1;
        
        if (setenv(name, value, 1) == -1) {
            perror("setenv failed");
        } else {
            printf("Set environment variable: %s=%s\n", name, value);
        }
    } else {
        printf("Error: Invalid format. Use -Vname=value\n");
    }
    
    free(copy);
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("No arguments provided. Use -h for help.\n");
        return 0;
    }
    
    printf("Processing options from right to left...\n\n");
    
    // Обрабатываем опции справа налево
    for (int i = argc - 1; i > 0; i--) {
        if (argv[i][0] == '-' && argv[i][1] != '\0') {
            char option = argv[i][1];
            char *value = &argv[i][2];
            
            switch (option) {
                case 'i':
                    print_user_ids();
                    break;
                case 's':
                    become_group_leader();
                    break;
                case 'p':
                    print_process_ids();
                    break;
                case 'u':
                    print_ulimit();
                    break;
                case 'U':
                    if (value[0] != '\0') {
                        change_ulimit(value);
                    } else {
                        printf("Error: -U requires a value (e.g., -U2048)\n");
                    }
                    break;
                case 'c':
                    print_core_size();
                    break;
                case 'C':
                    if (value[0] != '\0') {
                        change_core_size(value);
                    } else {
                        printf("Error: -C requires a value (e.g., -C500000)\n");
                    }
                    break;
                case 'd':
                    print_current_directory();
                    break;
                case 'v':
                    print_environment();
                    break;
                case 'V':
                    if (value[0] != '\0') {
                        set_environment_variable(value);
                    } else {
                        printf("Error: -V requires name=value (e.g., -VTEST=123)\n");
                    }
                    break;
                case 'h':
                    printf("=== Help ===\n");
                    printf("-i    Print user and group IDs\n");
                    printf("-s    Become group leader\n");
                    printf("-p    Print process IDs\n");
                    printf("-u    Print ulimit\n");
                    printf("-U#   Change ulimit (e.g., -U2048)\n");
                    printf("-c    Print core file size\n");
                    printf("-C#   Change core file size (e.g., -C500000)\n");
                    printf("-d    Print current directory\n");
                    printf("-v    Print environment variables\n");
                    printf("-Vn=v Set environment variable (e.g., -VTEST=123)\n");
                    printf("\n");
                    break;
                default:
                    printf("Unknown option: -%c\n", option);
                    printf("Use -h for help\n\n");
                    break;
            }
        }
    }
    
    return 0;
}
