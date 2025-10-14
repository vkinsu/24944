#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

volatile int timeout = 0;
int flag = 1;
void timeout_handler(int sig) { 
    timeout = 1; 
}

// Структура для хранения информации о строке
struct LineInfo {
    off_t start_offset;  // Начальное смещение строки
    off_t end_offset;    // Конечное смещение строки (начало следующей строки)
    size_t length;       // Длина строки
};

// Функция для вывода всего содержимого файла
void print_entire_file(int fd, struct LineInfo* lines, int countl) {
    printf("\nВремя истекло! Содержимое файла:\n");
    printf("==================================\n");
    for (int i = 0; i < countl; i++) {
        lseek(fd, lines[i].start_offset, SEEK_SET);

        char* output = malloc(lines[i].length + 1);
        read(fd, output, lines[i].length);
        output[lines[i].length] = '\0';

        printf("%s\n", output);
        free(output);
    }
    printf("==================================\n");
}

int main() {
    int fd = open("input.txt", O_RDONLY);
    char buffer[1024];
    int reader = read(fd, buffer, sizeof(buffer));

    // Подсчет количества строк
    int countl = 0;
    for (int i = 0; i < reader; i++) {
        if (buffer[i] == '\n') { countl++; }
    }
    if (reader > 0 && buffer[reader - 1] != '\n') { countl++; }  // Если файл не заканчивается символом новой строки, добавляем последнюю строку

    // Выделяем память под массив структур
    struct LineInfo* lines = malloc(countl * sizeof(struct LineInfo));

    // Заполняем структуры данными о строках
    int line_cur = 0;
    off_t line_start = 0;

    for (int i = 0; i < reader; i++) {
        if (buffer[i] == '\n' || i == reader - 1) {
            lines[line_cur].start_offset = line_start;
            lines[line_cur].end_offset = i + 1;
            lines[line_cur].length = i - line_start;

            // Для последней строки без символа новой строки
            if (i == reader - 1 && buffer[i] != '\n') {
                lines[line_cur].length = i - line_start + 1;
            }

            line_start = i + 1;
            line_cur++;
        }
    }

    // Вывод таблицы соответствия
    printf("== Таблица соответствия строки и номера ==\n");
    for (int j = 0; j < countl; j++) {
        printf("%d -> ", j + 1);

        lseek(fd, lines[j].start_offset, SEEK_SET);

        int line_length = lines[j].length;
        char* output = malloc(line_length + 1);

        read(fd, output, line_length);
        output[line_length] = '\0';
        printf("%s\n", output);
        free(output);
    }

    // Устанавливаем обработчик сигнала
    signal(SIGALRM, timeout_handler);

    // Основной цикл запросов
    while (1) {
        printf("Введите номер строки (1-%d, 0 для выхода): ", countl);
        fflush(stdout);  // Сбрасываем буфер вывода

        timeout = 0;
        alarm(5);  // Запускаем таймер на 5 секунд
        if(flag == 0) { alarm(0); }
        int n;
        int result = scanf("%d", &n);
        alarm(0);  // Отменяем таймер после ввода

        if (timeout & flag) {
            // Время истекло
            print_entire_file(fd, lines, countl);
            break;
        }

        if (result != 1) {
            // Ошибка ввода или конец файла
            if (result == EOF) {
                printf("\nКонец ввода.\n");
            } else {
                printf("Некорректный ввод. Пожалуйста, введите число.\n");
                // Очищаем буфер ввода
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                flag = 0;
            }
            continue;
        }

        if (n == 0) { 
            break; 
        }

        if (n < 1 || n > countl) {
            printf("Некорректный номер строки. Допустимый диапазон: 1-%d\n", countl);
            flag = 0;
            continue;
        }

        // Получаем информацию о запрошенной строке
        struct LineInfo line = lines[n - 1];
        lseek(fd, line.start_offset, SEEK_SET);

        int line_length = line.length;
        char* output = malloc(line_length + 1);

        read(fd, output, line_length);
        output[line_length] = '\0';
        printf("Строка %d: %s\n", n, output);
        flag = 0;
        free(output);
    }

    // Освобождение ресурсов
    free(lines);
    close(fd);

    return 0;
}