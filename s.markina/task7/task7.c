#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

volatile int timeout = 0;

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
void print_entire_file(char* file_data, struct LineInfo* lines, int countl) {
    printf("\nВремя истекло! Содержимое файла:\n");
    printf("==================================\n");
    for (int i = 0; i < countl; i++) {
        // Выводим строку напрямую из отображенной памяти
        printf("%.*s\n", (int)lines[i].length, file_data + lines[i].start_offset);
    }
    printf("==================================\n");
}

int main() {
    int fd = open("input.txt", O_RDONLY);

    // Получаем размер файла
    struct stat sb;
    size_t file_size = sb.st_size;
    if (file_size == 0) {
        printf("Файл пуст\n");
        close(fd);
        return 0;
    }

    // Отображаем файл в память
    char* file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

    // Подсчет количества строк
    int countl = 0;
    for (int i = 0; i < file_size; i++) {
        if (file_data[i] == '\n') { countl++; }
    }
    if (file_size > 0 && file_data[file_size - 1] != '\n') { countl++; }

    // Выделяем память под массив структур
    struct LineInfo* lines = malloc(countl * sizeof(struct LineInfo));


    // Заполняем структуры данными о строках
    int line_cur = 0;
    off_t line_start = 0;

    for (int i = 0; i < file_size; i++) {
        if (file_data[i] == '\n' || i == file_size - 1) {
            lines[line_cur].start_offset = line_start;
            lines[line_cur].end_offset = i + 1;
            lines[line_cur].length = i - line_start;

            // Для последней строки без символа новой строки
            if (i == file_size - 1 && file_data[i] != '\n') {
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
        // Выводим строку напрямую из отображенной памяти
        printf("%.*s\n", (int)lines[j].length, file_data + lines[j].start_offset);
    }

    // Устанавливаем обработчик сигнала
    signal(SIGALRM, timeout_handler);

    // Основной цикл запросов
    while (1) {
        printf("Введите номер строки (1-%d, 0 для выхода): ", countl);
        fflush(stdout);  // Сбрасываем буфер вывода

        timeout = 0;
        alarm(5);  // Запускаем таймер на 5 секунд

        int n;
        int result = scanf("%d", &n);

        alarm(0);  // Отменяем таймер после ввода

        if (timeout) {
            // Время истекло
            print_entire_file(file_data, lines, countl);
            break;
        }

        if (result != 1) {
            // Ошибка ввода или конец файла
            if (result == EOF) {
                printf("\nКонец ввода.\n");
            }
            else {
                printf("Некорректный ввод. Пожалуйста, введите число.\n");
                // Очищаем буфер ввода
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
            }
            continue;
        }

        if (n == 0) {
            break;
        }

        if (n < 1 || n > countl) {
            printf("Некорректный номер строки. Допустимый диапазон: 1-%d\n", countl);
            continue;
        }

        // Получаем информацию о запрошенной строке
        struct LineInfo line = lines[n - 1];
        // Выводим строку напрямую из отображенной памяти
        printf("Строка %d: %.*s\n", n, (int)line.length, file_data + line.start_offset);
    }

    // Освобождение ресурсов
    free(lines);
    munmap(file_data, file_size);
    close(fd);

    return 0;
}