// result1-mini.c — -u показывает лимит процессов (ulimit -u)
// с фолбэком через sysconf(_SC_CHILD_MAX)
#define _GNU_SOURCE
#define _XOPEN_SOURCE 700
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

extern char **environ;

static long num(const char *s){ char *e; errno=0; long v=strtol(s,&e,10); return (!errno && *e==0)?v:-1; }
static void use(const char *p){ fprintf(stderr,"usage: %s [-i] [-s] [-p] [-u] [-U n] [-c] [-C n] [-d] [-v] [-V name=val]\n",p); }

static void print_nproc(void){
#ifdef RLIMIT_NPROC
    struct rlimit r; 
    if (getrlimit(RLIMIT_NPROC,&r)==0){ printf("nproc=%ld\n",(long)r.rlim_cur); return; }
#endif
    long v = sysconf(_SC_CHILD_MAX);
    if (v > 0) printf("nproc=%ld\n", v);
    else perror("nproc");
}

static int set_nproc(long v){
#ifdef RLIMIT_NPROC
    struct rlimit r;
    if (getrlimit(RLIMIT_NPROC,&r)==-1) return perror("getrlimit"), -1;
    r.rlim_cur = (rlim_t)v;
    if (setrlimit(RLIMIT_NPROC,&r)==-1) return perror("setrlimit"), -1;
    return 0;
#else
    fprintf(stderr,"set nproc: unsupported on this system\n");
    return -1;
#endif
}

int main(int ac, char **av){
    if (ac<2){ use(av[0]); return 2; }

    int o; struct rlimit r;
    while ((o=getopt(ac,av,"ispuU:cC:dvV:"))!=-1){
        switch(o){
        case 'i':
            printf("uid=%ld euid=%ld gid=%ld egid=%ld\n",
                   (long)getuid(),(long)geteuid(),(long)getgid(),(long)getegid());
            break;
        case 's':
            if (setpgid(0,0)==-1) perror("setpgid");
            break;
        case 'p':
            printf("pid=%ld ppid=%ld pgid=%ld\n",
                   (long)getpid(),(long)getppid(),(long)getpgid(0));
            break;
        case 'u':
            print_nproc();
            break;
        case 'U': {
            long v=num(optarg); if (v<0){ fputs("-U bad\n",stderr); return 3; }
            set_nproc(v);
        } break;
        case 'c':
            if (getrlimit(RLIMIT_CORE,&r)==0) printf("core=%ld\n",(long)r.rlim_cur);
            else perror("getrlimit");
            break;
        case 'C': {
            long v=num(optarg); if (v<0){ fputs("-C bad\n",stderr); return 3; }
            if (getrlimit(RLIMIT_CORE,&r)==0){ r.rlim_cur=(rlim_t)v; if (setrlimit(RLIMIT_CORE,&r)==-1) perror("setrlimit"); }
            else perror("getrlimit");
        } break;
        case 'd': {
            char *cwd=getcwd(NULL,0);
            if (!cwd) perror("getcwd"); else { printf("cwd=%s\n",cwd); free(cwd); }
        } break;
        case 'v':
            for (char **p=environ; *p; ++p) puts(*p);
            break;
        case 'V':
            if (!strchr(optarg,'=')) { fputs("-V name=val\n",stderr); return 4; }
            if (putenv(optarg)!=0) perror("putenv");
            break;
        default:
            use(av[0]); return 1;
        }
    }
    return 0;
}
