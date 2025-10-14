#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

volatile bool timeout = false;

void timeout_handler(int sig) { timeout = true; }

int main()
{
    int st = open("input.txt", O_RDONLY);

    char buffer[1024];
    int read_bites = read(st, buffer, sizeof(buffer));

    int count_str = 0;
    for (int i = 0; i < read_bites; i++) { if (buffer[i] == '\n') { count_str++;} }

    if (buffer[read_bites-1] != '\n') { count_str++; }

    int pos_str[count_str + 1];
    pos_str[0] = 0;
    int idx = 1;

    for (int i = 0; i < read_bites; i++)
    {
        if (buffer[i] == '\n') 
        {
            pos_str[idx] = i + 1;
            idx++;
        }
    }

    pos_str[count_str] = read_bites;

    printf("== РўР°Р±Р»РёС†Р° СЃРѕРѕС‚РІРµС‚СЃС‚РІРёСЏ СЃС‚СЂРѕРєРё Рё РЅРѕРјРµСЂР° ==\n");
    for (int j = 0; j < count_str; j++)
    {
        lseek(st, pos_str[j], 0);

        int index = pos_str[j+1] - pos_str[j];
        char output[index];
        read(st, output, index);

	printf("%d	%d	%d", j+1, pos_str[j], index);

        printf("\n");
    }

    signal(SIGALRM, timeout_handler);
    int flag = 0;

    while(1)
    {
	printf("Р’РІРµРґРёС‚Рµ РЅРѕРјРµСЂ СЃС‚СЂРѕРєРё (1-%d): ", count_str);
	fflush(stdout);

	if (flag == 0)
	{
		timeout = false;
        	alarm(5);
		flag = 1;
	}
	
        int n;
        int result = scanf("%d", &n);

	alarm(0);
	
	if (timeout) {
            printf("\nВывод всего файла:\n");
            for (int i = 0; i < read_bites; i++) { printf("%c", buffer[i]); }
	    printf("\n");
            break;
        }

	if (result != 1)
	{
		printf("Invalid index2\n");
		int c;
        	while ((c = getchar()) != '\n' && c != EOF);
		continue;
	}

        if (n == 0) { break; }
	else if (n > count_str || n < 1) { printf("Invalid index\n"); continue; }

        if (result == 1)
        {
            lseek(st, pos_str[n-1], SEEK_SET);

            int index = pos_str[n] - pos_str[n-1];
            char output[index];
            read(st, output, index);

            for (int i = 0; i < index; i++) { printf("%c", output[i]); }
	    printf("\n");
        }
    }

    close(st);

    return 0;
}
