#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern char *tzname[];

int main(void)
{
    time_t now = time(NULL);

    /* В Solaris подойдут "US/Pacific", "America/Los_Angeles" или "PST8PDT" */
    if (setenv("TZ", "America/Los_Angeles", 1) != 0) {
        /* запасной вариант */
        setenv("TZ", "PST8PDT", 1);
    }
    tzset();  /* применить TZ для последующих вызовов */

    struct tm *sp = localtime(&now);

    printf("%s", ctime(&now));  /* строка в текущей TZ (теперь PT) */
    printf("%d/%d/%02d %02d:%02d %s\n",
           sp->tm_mon + 1,         /* месяц [1..12] */
           sp->tm_mday,            /* день */
           sp->tm_year + 1900,     /* полный год */
           sp->tm_hour,            /* часы */
           sp->tm_min,             /* минуты */
           tzname[sp->tm_isdst]);  /* аббревиатура TZ: PST/PDT */

    return 0;
}
