#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/mman.h>

struct Stroka{
    off_t otstup;
    ssize_t len;
};

volatile sig_atomic_t timeout_occurred = 0;

void timeout_handler(int sig) {
    timeout_occurred = 1;
}

void print_entire_file(char *file_data, size_t file_size) {
    printf("\nВремя вышло! Вывод всего содержимого файла:\n");
    printf("============================================\n");
    
    // Просто выводим все данные файла из памяти
    write(STDOUT_FILENO, file_data, file_size);
    
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
    
    // Получаем размер файла
    struct stat sb;
    if (fstat(file, &sb) == -1) {
        perror("Ошибка получения информации о файле");
        close(file);
        exit(1);
    }
    size_t file_size = sb.st_size;
    
    // Отображаем файл в память
    char *file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, file, 0);
    if (file_data == MAP_FAILED) {
        perror("Ошибка отображения файла в память");
        close(file);
        exit(1);
    }
    
    // Построение таблицы строк с использованием отображенной памяти
    off_t now_otstup = 0;
    int stroka_count = 0;
    
    for (size_t i = 0; i < file_size; i++) {
        if (file_data[i] == '\n') {
            stroki[stroka_count].otstup = now_otstup;
            stroki[stroka_count].len = i - now_otstup;
            stroka_count++;
            printf("Строка %d: offset=%ld, length=%ld\n", stroka_count, 
                   (long)stroki[stroka_count-1].otstup, 
                   (long)stroki[stroka_count-1].len);
            now_otstup = i + 1; // Следующая строка начинается после '\n'
        }
    }
    
    // Обработка последней строки, если файл не заканчивается на '\n'
    if (now_otstup < file_size) {
        stroki[stroka_count].otstup = now_otstup;
        stroki[stroka_count].len = file_size - now_otstup;
        stroka_count++;
        printf("Строка %d: offset=%ld, length=%ld\n", stroka_count, 
               (long)stroki[stroka_count-1].otstup, 
               (long)stroki[stroka_count-1].len);
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
            print_entire_file(file_data, file_size);
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
            // Получаем данные строки напрямую из отображенной памяти
            off_t offset = stroki[nomer - 1].otstup;
            ssize_t length = stroki[nomer - 1].len;
            
            // Создаем буфер для строки и копируем данные
            char line_buffer[length + 1];
            memcpy(line_buffer, file_data + offset, length);
            line_buffer[length] = '\0';
            
            printf(">>> Строка %d: '%s'\n\n", nomer, line_buffer);
        }
    }
    
    // Освобождаем отображенную память
    if (munmap(file_data, file_size) == -1) {
        perror("Ошибка при освобождении отображенной памяти");
    }
    
    if (close(file) == -1) {
        perror("Ошибка при закрытии файла");
        exit(1);
    }

    return 0;
}