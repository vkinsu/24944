#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(void)
{
       time_t now;
       struct tm *local_time, *california_time;
       time(&now);

       local_time = localtime(&now);
       printf("Local time: %02d/%02d/%04d %02d:%02d:%02d\n",
              local_time->tm_mon + 1,
              local_time->tm_mday,
              local_time->tm_year + 1900,
              local_time->tm_hour,
              local_time->tm_min,
              local_time->tm_sec);

       putenv("TZ=America/Los_Angeles");
       tzset();
       california_time = localtime(&now);

       printf("California time: %02d/%02d/%04d %02d:%02d:%02d %s\n",
              california_time->tm_mon + 1,
              california_time->tm_mday,
              california_time->tm_year + 1900,
              california_time->tm_hour,
              california_time->tm_min,
              california_time->tm_sec,
              california_time->tm_isdst ? "(DST, UTC-7)" : "(STD, UTC-8)");

       putenv("TZ=");
       tzset();

       return 0;
}
