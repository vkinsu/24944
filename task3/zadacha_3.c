#include <stdio.h>
#include <errno.h>
#include <unistd.h>


void print_ids(){
    printf("Real: %d\n", getuid());
    printf("Effective: %d\n", geteuid());
}

int main()
{
    print_ids();
    FILE *file = fopen("x_txt", "r");
    if(file == NULL){
        perror("Невозможно открыть файл");
    }
    else{
        fclose(file);
    }

    if (setuid(getuid()) == -1) {
        perror("Ошибка: не удалось установить UID");
    }

    print_ids();
    FILE *file = fopen("x_txt", "r");
    if(file == NULL){
        perror("Невозможно открыть файл");
    }
    else{
        fclose(file);
    }
}