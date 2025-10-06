#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/resource.h>
#include <errno.h>

const char* get_resource_name(int resource) {
    switch (resource) {
        case RLIMIT_AS:         return "Virtual memory";
        case RLIMIT_CORE:       return "Core file size";
        case RLIMIT_CPU:        return "CPU time";
        case RLIMIT_DATA:       return "Data segment";
        case RLIMIT_FSIZE:      return "File size";
        case RLIMIT_LOCKS:      return "File locks";
        case RLIMIT_MEMLOCK:    return "Locked memory";
        case RLIMIT_MSGQUEUE:   return "Message queues";
        case RLIMIT_NICE:       return "Nice value";
        case RLIMIT_NOFILE:     return "Open files";
        case RLIMIT_NPROC:      return "Processes";
        case RLIMIT_RSS:        return "Resident set";
        case RLIMIT_RTPRIO:     return "Real-time priority";
        case RLIMIT_RTTIME:     return "Real-time timeout";
        case RLIMIT_SIGPENDING: return "Pending signals";
        case RLIMIT_STACK:      return "Stack size";
        default:                return "Unknown";
    }
}

void print_limit(int resource) {
    struct rlimit limit;
    const char *name = get_resource_name(resource);
    
    if (getrlimit(resource, &limit) == 0) {
        if (limit.rlim_cur == RLIM_INFINITY) {
            printf("%-20s: soft=unlimited, hard=unlimited\n", name);
        } else {
            printf("%-20s: soft=%10ld, hard=%10ld\n", 
                   name, (long)limit.rlim_cur, (long)limit.rlim_max);
        }
    } else {
        printf("%-20s: error: %s\n", name, strerror(errno));
    }
}

void print_all_limits() {
    printf("Resource limits for PID %d:\n", getpid());
    printf("=============================================\n");
    
    print_limit(RLIMIT_AS);
    print_limit(RLIMIT_CORE);
    print_limit(RLIMIT_CPU);
    print_limit(RLIMIT_DATA);
    print_limit(RLIMIT_FSIZE);
    print_limit(RLIMIT_LOCKS);
    print_limit(RLIMIT_MEMLOCK);
    print_limit(RLIMIT_MSGQUEUE);
    print_limit(RLIMIT_NICE);
    print_limit(RLIMIT_NOFILE);
    print_limit(RLIMIT_NPROC);
    print_limit(RLIMIT_RSS);
    print_limit(RLIMIT_RTPRIO);
    print_limit(RLIMIT_RTTIME);
    print_limit(RLIMIT_SIGPENDING);
    print_limit(RLIMIT_STACK);
}

void print_usage(const char *program_name) {
    printf("Usage: %s [OPTION]\n", program_name);
    printf("Options:\n");
    printf("  -i, --ids       Print real and effective IDs\n");
    printf("  -s, --setpgid   Become process group leader\n");
    printf("  -p, --process   Print process IDs (PID, PPID)\n");
    printf("  -u, --ulimit    Print all resource limits\n");
    printf("  -h, --help      Display this help message\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--ids") == 0) {
        printf("Real GID: %d\n", getgid());
        printf("Effective GID: %d\n", getegid());
        printf("Real UID: %d\n", getuid());
        printf("Effective UID: %d\n", geteuid());
    } 
    else if (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--setpgid") == 0) {
        if (setpgid(0, 0) == -1) {
            perror("setpgid failed");
            return 1;
        }
        printf("Process became group leader: PID=%d, PGID=%d\n", getpid(), getpgrp());
    }
    else if (strcmp(argv[1], "-p") == 0 || strcmp(argv[1], "--process") == 0) {
        printf("PID: %d\n", getpid());
        printf("PPID: %d\n", getppid());
        printf("PGID: %d\n", getpgrp());
        printf("SID: %d\n", getsid(0));
    }
    else if (strcmp(argv[1], "-u") == 0 || strcmp(argv[1], "--ulimit") == 0) {
        print_all_limits();
    }
    else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
    }
    else {
        printf("Unknown option: %s\n", argv[1]);
        print_usage(argv[0]);
        return 1;
    }
    
    return 0;
}