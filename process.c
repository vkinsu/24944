#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

extern char **environ;

// ==== helpers ====
static void print_user_ids(void){
    printf("=== User and Group IDs ===\n");
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
    printf("Real GID: %d\n", getgid());
    printf("Effective GID: %d\n\n", getegid());
}
static void become_group_leader(void){
    printf("=== Becoming Group Leader ===\n");
    if (setpgid(0,0)==-1) perror("setpgid failed");
    else {
        printf("Process became group leader successfully\n");
        printf("New Group ID: %d\n\n", getpgrp());
    }
}
static void print_process_ids(void){
    printf("=== Process IDs ===\n");
    printf("PID: %d\nPPID: %d\nPGID: %d\n\n", getpid(), getppid(), getpgrp());
}
static void show_rlimit(int res, const char *label){
    struct rlimit rl;
    if (getrlimit(res,&rl)==-1){ perror("getrlimit"); return; }
    printf("=== %s ===\n", label);
    if (rl.rlim_cur==RLIM_INFINITY) printf("cur = unlimited\n");
    else printf("cur = %llu bytes\n", (unsigned long long)rl.rlim_cur);
    if (rl.rlim_max==RLIM_INFINITY) printf("max = unlimited\n\n");
    else printf("max = %llu bytes\n\n", (unsigned long long)rl.rlim_max);
}
static int parse_bytes(const char *s, rlim_t *out){
    errno=0; char *end=NULL; unsigned long long v=strtoull(s,&end,10);
    if (errno || end==s || *end!='\0') return -1;
    *out=(rlim_t)v; return 0;
}
static void set_rlimit_val(int res, const char *name, const char *arg){
    rlim_t v;
    if (parse_bytes(arg,&v)!=0){ fprintf(stderr,"Invalid value for %s: %s\n", name, arg); return; }
    struct rlimit rl;
    if (getrlimit(res,&rl)==-1){ perror("getrlimit"); return; }
    rl.rlim_cur=v;
    if (setrlimit(res,&rl)==-1) perror("setrlimit");
    else printf("=== %s updated to %llu bytes ===\n\n", name, (unsigned long long)v);
}
static void print_cwd(void){
    char buf[PATH_MAX];
    if (!getcwd(buf,sizeof(buf))) { perror("getcwd"); return; }
    printf("=== Current Working Directory ===\n%s\n\n", buf);
}
static void print_env_all(void){
    printf("=== Environment Variables ===\n");
    for (char **e=environ; e && *e; ++e) puts(*e);
    printf("\n");
}
static void set_env_kv(const char *kv){
    char *dup=strdup(kv); if(!dup){ perror("strdup"); return; }
    char *eq=strchr(dup,'=');
    if (!eq || eq==dup || !*(eq+1)){
        fprintf(stderr,"Invalid -V, expected name=value: %s\n", kv);
        free(dup); return;
    }
    *eq='\0'; const char *name=dup; const char *val=eq+1;
    if (setenv(name,val,1)==-1) perror("setenv");
    else printf("=== ENV set: %s=%s ===\n\n", name, getenv(name));
    free(dup);
}

// ==== main: manual right-to-left parser with clusters and args ====
int main(int argc, char *argv[]){
    printf("Processing options from right to left...\n\n");

    for (int i = argc - 1; i > 0; --i){
        const char *arg = argv[i];
        if (arg[0] != '-' || arg[1] == '\0') continue; // not an option

        // iterate over a cluster: e.g. "-ipd"
        for (int k = 1; arg[k] != '\0'; ++k){
            char opt = arg[k];

            // helpers to fetch argument for options that require one
            auto get_inline = [&](void)->const char* {
                return (arg[k+1] != '\0') ? &arg[k+1] : NULL; // rest of this token
            };
            auto get_next = [&](void)->const char* {
                if (i+1 < argc && argv[i+1][0] != '-') return argv[i+1];
                return NULL;
            };

            switch (opt){
                case 'i': print_user_ids(); break;
                case 's': become_group_leader(); break;
                case 'p': print_process_ids(); break;
                case 'u': show_rlimit(RLIMIT_FSIZE, "ULIMIT (RLIMIT_FSIZE)"); break;
                case 'c': show_rlimit(RLIMIT_CORE,  "CORE (RLIMIT_CORE)"); break;
                case 'd': print_cwd(); break;
                case 'v': print_env_all(); break;

                case 'U': {
                    const char *val = get_inline();
                    if (!val) val = get_next();
                    if (!val){ fprintf(stderr,"-U requires bytes\n"); break; }
                    set_rlimit_val(RLIMIT_FSIZE, "ULIMIT", val);
                    // if inline was used, stop processing the rest of this token
                    if (arg[k+1] != '\0') k = (int)strlen(arg)-1;
                } break;

                case 'C': {
                    const char *val = get_inline();
                    if (!val) val = get_next();
                    if (!val){ fprintf(stderr,"-C requires bytes\n"); break; }
                    set_rlimit_val(RLIMIT_CORE, "CORE", val);
                    if (arg[k+1] != '\0') k = (int)strlen(arg)-1;
                } break;

                case 'V': {
                    const char *kv = get_inline();
                    if (!kv) kv = get_next();
                    if (!kv){ fprintf(stderr,"-V requires name=value\n"); break; }
                    set_env_kv(kv);
                    if (arg[k+1] != '\0') k = (int)strlen(arg)-1;
                } break;

                default:
                    printf("Unknown option: -%c\n", opt);
                    break;
            }

            // если это была опция с аргументом в остатке токена, мы уже выставили k на конец токена
        }
    }
    return 0;
}
