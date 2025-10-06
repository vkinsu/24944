#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];
    
    setenv("TZ", "PST8PDT", 1);
    tzset();
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    
    strftime(buffer, sizeof(buffer), "Текущее время в Калифорнии (PST): %Y-%m-%d %H:%M:%S %Z", timeinfo);
    printf("%s\n", buffer);
    
    return 0;
}