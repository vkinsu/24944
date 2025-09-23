#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(void) {
    time_t now = time(NULL);

    // Устанавливаем временную зону Калифорнии (Лос-Анджелес)
    setenv("TZ", "America/Los_Angeles", 1);
    tzset();

    // Вывод времени в Калифорнии
    printf("Калифорния: %s", ctime(&now));

    struct tm *tm_cal = localtime(&now);
    printf("%02d/%02d/%04d %02d:%02d\n",
           tm_cal->tm_mon + 1,
           tm_cal->tm_mday,
           tm_cal->tm_year + 1900,
           tm_cal->tm_hour,
           tm_cal->tm_min);

    return 0;
}
