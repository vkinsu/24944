#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(void) {
    time_t t;
    struct tm *tm;

    setenv("TZ", "PST8", 1);
    tzset();

    time(&t);

    tm = localtime(&t);

    printf("%s", asctime(tm)); 
    printf("%02d/%02d/%d %02d:%02d:%02d PST\n",
           tm->tm_mon + 1,
           tm->tm_mday,
           tm->tm_year + 1900,
           tm->tm_hour,
           tm->tm_min,
           tm->tm_sec);

    return 0;
}
