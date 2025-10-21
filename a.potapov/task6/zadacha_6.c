#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>

struct Stroka{
    off_t otstup;
    ssize_t len;
};

volatile sig_atomic_t timeout_occurred = 0;

void timeout_handler(int sig) {
    timeout_occurred = 1;
}

void print_entire_file(int file) {
    printf("\nВремя вышло! Вывод всего содержимого файла:\n");
    printf("============================================\n");
    
    if (lseek(file, 0, SEEK_SET) == -1) {
        perror("Ошибка позиционирования в файле");
        return;
    }
    
    char buffer[1024];
    ssize_t bytes_read;
    
    while ((bytes_read = read(file, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }
    
    printf("\n============================================\n");
}

int main(int argc, char *argv[]){
    struct Stroka stroki[1000];
    
    // Установка обработчика сигнала
    if (signal(SIGALRM, timeout_handler) == SIG_ERR) {
        perror("Ошибка установки обработчика сигнала");
        exit(1);
    }
    
    if(argc != 2){
        perror ("Нужен один аргумент");
        exit(1);
    }
    
    int file = open(argv[1], O_RDONLY);
    if(file == -1){
        perror("Невозможно открыть файл");
        exit(1);
    }
    
    off_t now_otstup = 0;
    int stroka_count = 0;
    char buffer[1024];
    ssize_t bytes_read = 0;
    
    // Чтение файла и построение таблицы строк
    while ((bytes_read = read(file, buffer, 1024)) > 0) {
        for(ssize_t i = 0; i < bytes_read; i++){
            if(buffer[i] == '\n'){
                off_t now_pos = lseek(file, 0L, SEEK_CUR);
                stroki[stroka_count].otstup  = now_otstup;
                stroki[stroka_count].len = now_pos - now_otstup - (bytes_read - i);
                stroka_count++;
                printf("Строка %d: offset=%ld, length=%ld\n", stroka_count, (long)stroki[stroka_count-1].otstup, (long)stroki[stroka_count-1].len);
                now_otstup = now_pos - (bytes_read - i - 1);
            }
        }
    }
    
    off_t file_size = lseek(file, 0L, SEEK_END);
    if (now_otstup < file_size) {
        stroki[stroka_count].otstup = now_otstup;
        stroki[stroka_count].len = file_size - now_otstup;
        stroka_count++;
        printf("Строка %d: offset=%ld, length=%ld\n", stroka_count, (long)stroki[stroka_count-1].otstup, (long)stroki[stroka_count-1].len);
    }

    // Интерактивный режим с таймаутом
    while(1){
        printf("Введите номер строки и её покажут (0 для завершения программы):\n");
        printf("У вас 5 секунд на ввод...\n");
        
        // Установка таймера на 5 секунд
        timeout_occurred = 0;
        alarm(5);
        
        int nomer;
        int result = scanf("%d", &nomer);
        
        // Отмена таймера
        alarm(0);
        
        // Проверка таймаута
        if (timeout_occurred) {
            print_entire_file(file);
            break;
        }
        
        if(result != 1){
            printf("Ошибка ввода, попробуйте еще раз\n");
            // Очистка буфера ввода
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        
        if(nomer == 0){
            printf("Завершение программы...\n");
            break;
        }
        else if(nomer < 1 || nomer > stroka_count){
            printf("Неверный номер, попробуй ещё раз\n");
        }
        else{
            if (lseek(file, stroki[nomer - 1].otstup, SEEK_SET) == -1) {
                perror("Ошибка позиционирования в файле");
                continue;
            }
            char line_buffer[stroki[nomer - 1].len + 1];
            ssize_t read_bytes = read(file, line_buffer, stroki[nomer - 1].len);
            if (read_bytes == -1) {
                perror("Ошибка чтения строки");
                continue;
            }
            line_buffer[read_bytes] = '\0';
            printf(">>> Строка %d: '%s'\n\n", nomer, line_buffer);
        }
    }
    
    if (close(file) == -1) {
        perror("Ошибка при закрытии файла");
        exit(1);
    }

    return 0;
}