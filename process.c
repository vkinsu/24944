#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

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

int main(int argc, char *argv[]) {
    int opt;
    
    printf("Processing options from right to left...\n\n");
    
    // Альтернативная обработка опций (простая версия для Linux)
    for (int i = argc - 1; i > 0; i--) {
        if (argv[i][0] == '-' && argv[i][1] != '\0') {
            char option = argv[i][1];
            
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
                default:
                    printf("Unknown option: -%c\n", option);
                    break;
            }
        }
    }
    
    return 0;
}
