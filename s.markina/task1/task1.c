#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <getopt.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/resource.h>
extern char **environ;


int main(int argc, char *argv[]) {
    char options[] = "ispudU:cC:vV:";
    int c, invalid = 0, iflg = 0, sflg = 0, pflg = 0, dflg = 0, 
        uflg = 0, Uflg = 0, cflg = 0, Cflg = 0, vflg = 0, Vflg = 0;
    extern char *optarg;
    extern int optind, opterr, optopt;
    struct rlimit rlp;
    char cwd[1024];  

    printf("argc equals %d\n", argc);
    
    // Обрабатываем опции справа налево 
    while ((c = getopt(argc, argv, options)) != -1) {  
        switch(c) {
            case 'i':
                iflg++;
                printf("=== Option -i ===\n");
                printf("Real User ID = %d\n", getuid());
                printf("Effective User ID = %d\n", geteuid());
                printf("Real Group ID = %d\n", getgid());
                printf("Effective Group ID = %d\n", getegid());
                printf("\n");
                break;
                
            case 's':
                sflg++;
                printf("=== Option -s ===\n");
                if (setpgid(0, getpid()) == 0) { 
                    printf("Process became group leader successfully\n");
                } else {
                    perror("Failed to set process group");
                }
                printf("\n");
                break;
                
            case 'p':
                pflg++;
                printf("=== Option -p ===\n");
                printf("Process ID (pid) = %d\n", getpid());
                printf("Parent Process ID (ppid) = %d\n", getppid());
                printf("Process Group ID (pgid) = %d\n", getpgid(0));
                printf("\n");
                break;
                
            case 'u':
                uflg++;
                printf("=== Option -u ===\n");
                if (getrlimit(RLIMIT_NPROC, &rlp) == 0) {  
                    printf("File size limit (ulimit):\n");
                    printf("  Soft limit: %ld\n", rlp.rlim_cur);
                    printf("  Hard limit: %ld\n", rlp.rlim_max);
                } else {
                    perror("Failed to get file size limit");
                }
                printf("\n");
                break;
                
            case 'U':
                Uflg++;
                printf("=== Option -U %s ===\n", optarg);
                if (getrlimit(RLIMIT_FSIZE, &rlp) == 0) {
                    rlp.rlim_cur = atol(optarg);
                    if (setrlimit(RLIMIT_FSIZE, &rlp) == -1) {
                        perror("Failed to change ulimit");
                    } else {
                        printf("Ulimit changed to: %ld\n", rlp.rlim_cur);
                    }
                } else {
                    perror("Failed to get current ulimit");
                }
                printf("\n");
                break;
                
            case 'c':
                cflg++;
                printf("=== Option -c ===\n");
                if (getrlimit(RLIMIT_CORE, &rlp) == 0) {
                    printf("Core file size limit:\n");
                    printf("  Soft limit: %ld bytes\n", rlp.rlim_cur);
                    printf("  Hard limit: %ld bytes\n", rlp.rlim_max);
                } else {
                    perror("Failed to get core file limit");
                }
                printf("\n");
                break;
                
            case 'C':
                Cflg++;
                printf("=== Option -C %s ===\n", optarg);
                if (getrlimit(RLIMIT_CORE, &rlp) == 0) {
                    rlp.rlim_cur = atol(optarg);
                    if (setrlimit(RLIMIT_CORE, &rlp) == -1) {
                        perror("Failed to change core file size");
                    } else {
                        printf("Core file size changed to: %ld bytes\n", rlp.rlim_cur);
                    }
                } else {
                    perror("Failed to get current core file limit");
                }
                printf("\n");
                break;
                
            case 'd':
                dflg++;
                printf("=== Option -d ===\n");
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("Current working directory: %s\n", cwd);
                } else {
                    perror("Failed to get current directory");
                }
                printf("\n");
                break;
                
            case 'v':
                vflg++;
                printf("=== Option -v ===\n");
                printf("Environment variables:\n");
                for (char** env = environ; *env != NULL; env++) {
                    printf("  %s\n", *env);
                }
                printf("\n");
                break;
                
            case 'V':
                Vflg++;
                printf("=== Option -V %s ===\n", optarg);
                if (putenv(optarg) == 0) {
                    printf("Environment variable set successfully: %s\n", optarg);
                } else {
                    perror("Failed to set environment variable");
                }
                printf("\n");
                break;
                
            case '?':
                invalid++;
                fprintf(stderr, "Unknown option: -%c\n", optopt);
                break;
        }
    }

    // информация
    printf("=== Debug Information ===\n");
    printf("iflg (-i): %d\n", iflg);
    printf("sflg (-s): %d\n", sflg);
    printf("pflg (-p): %d\n", pflg);
    printf("uflg (-u): %d\n", uflg);
    printf("Uflg (-U): %d\n", Uflg);
    printf("cflg (-c): %d\n", cflg);
    printf("Cflg (-C): %d\n", Cflg);
    printf("dflg (-d): %d\n", dflg);
    printf("vflg (-v): %d\n", vflg);
    printf("Vflg (-V): %d\n", Vflg);
    printf("invalid options: %d\n", invalid);
    printf("optind: %d\n", optind);
    
    if(optind < argc)
        printf("next parameter = %s\n", argv[optind]);    
    return 0;
}