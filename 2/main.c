#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>

extern char *tzname[];

int main()
{
    time_t n;
    (void) time(&n);

    putenv("TZ=PST8PDT");
    printf("%s", ctime(&n));
}