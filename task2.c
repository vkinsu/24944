

#include <stdio.h>
#include <time.h>

int main(void) {
    time_t now = time(NULL);          // текущее время в секундах с 1970-01-01 UTC
    time_t pst = now - 8 * 3600;      // сдвиг на -8 часов (PST = UTC-8)

    // Вывод как готовая строка
    printf("Калифорния (PST): %s", ctime(&pst));

    // Разложим и выведем вручную (MM/DD/YYYY HH:MM)
    struct tm *tm_pst = gmtime(&pst); // gmtime, потому что мы уже сдвинули на -8
    if (tm_pst) {
        printf("%02d/%02d/%04d %02d:%02d PST\n",
               tm_pst->tm_mon + 1,
               tm_pst->tm_mday,
               tm_pst->tm_year + 1900,
               tm_pst->tm_hour,
               tm_pst->tm_min);
    }
    return 0;
}
