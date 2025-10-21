#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    time_t now = time(NULL);
    if (now == (time_t)-1) {
        perror("time");
        return 1;
    }

    /* Устанавливаем часовой пояс и применяем его */
    if (setenv("TZ", "PST8PDT", 1) == -1) {
        perror("setenv");
        return 1;
    }
    tzset();

    /* Печать времени в текстовом формате ctime (включает '\n') */
    printf("%s", ctime(&now));

    /* Локальное время с разбивкой по полям */
    struct tm *sp = localtime(&now);
    if (!sp) {
        perror("localtime");
        return 1;
    }

    /* tm_year — годы с 1900, tm_mon — месяцы 0..11 */
    printf("%02d/%02d/%04d %02d:%02d %s\n",
           sp->tm_mon + 1,
           sp->tm_mday,
           sp->tm_year + 1900,
           sp->tm_hour,
           sp->tm_min,
           tzname[sp->tm_isdst ? 1 : 0]);

    return 0;
}
