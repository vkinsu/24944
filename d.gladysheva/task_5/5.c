#include <stdio.h>
#include <fcntl.h>

int main()
{
    // РґРµСЃРєСЂРёРїС‚РѕСЂ С„Р°Р№Р»Р°
    int st = open("input.txt", O_RDONLY);

    char buffer[1024];
    // РєРѕР»РёС‡РµСЃС‚РІРѕ РїСЂРѕС‡РёС‚Р°РЅРЅС‹С… Р±Р°Р№С‚
    int read_bites = read(st, buffer, sizeof(buffer));

    int count_str = 0; // РєРѕР»-РІРѕ СЃС‚СЂРѕРє
    for (int i = 0; i < read_bites; i++) { if (buffer[i] == '\n') { count_str++;} }

    if (buffer[read_bites-1] != '\n') { count_str++; }

    int pos_str[count_str + 1]; // РїРѕР·РёС†РёРё РЅР°С‡Р°Р»Р° СЃС‚СЂРѕРє
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
       
        // РґРµСЃРєСЂРёРїС‚РѕСЂ, СЃРјРµС‰РµРЅРёРµ, РѕС‚РєСѓРґР°
        lseek(st, pos_str[j], 0);

        int index = pos_str[j+1] - pos_str[j];
        char output[index];
        read(st, output, index);

         printf("%d	%d	%d", j+1, pos_str[j], index);
        printf("\n");
    }

    while(1)
    {
        printf("Р’РІРµРґРёС‚Рµ РЅРѕРјРµСЂ СЃС‚СЂРѕРєРё (1-%d): ", count_str);
        int n;
        int result = scanf("%d", &n);

        if (n == 0) { break; } 
	else if (n > count_str || n < 1)
	{
		printf("Invalid index\n");
		int c;
    		while ((c = getchar()) != '\n' && c != EOF);
		continue;
	}
	else if (result !=1)
	{
		printf("Error\n");
		int c;
    		while ((c = getchar()) != '\n' && c != EOF);
		continue;
	}

        lseek(st, pos_str[n-1], 0);

        int index = pos_str[n] - pos_str[n-1];
        char output[index];
        read(st, output, index);

        for (int i = 0; i < index; i++) { printf("%c", output[i]); }
    }

    close(st);

    return 0;
}
