#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    time_t rawtime;
    struct tm *timeinfo;
    
    setenv("TZ", "America/Los_Angeles", 1);
    tzset();
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    printf("%d/%d/%02d %d:%02d PST\n",
        timeinfo->tm_mon + 1, timeinfo->tm_mday,
        timeinfo->tm_year +1900, timeinfo->tm_hour - 1,
        timeinfo->tm_min);

    return 0;
}