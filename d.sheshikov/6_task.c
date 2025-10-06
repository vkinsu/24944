#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>

typedef struct {
    long offset;
    int length;
    int idx;
} LineInfo;

volatile sig_atomic_t time_out = 0;

void alarm_handler(int sig) {
    time_out = 1;
}

int nonblocking_scanf(int *number) {
    fd_set readfds;
    struct timeval timeout;
    
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    
    int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
    
    if (ret > 0) {
        return scanf("%d", number);
    }
    return 0;
}

LineInfo* build_line_table(int fd, int *line_count) {
    LineInfo *table = NULL;
    int capacity = 100;
    int count = 0;
    char ch;
    long current_offset = 0;
    long line_start = 0;
    
    table = malloc(capacity * sizeof(LineInfo));    
    
    lseek(fd, 0L, SEEK_SET);
    
    while (read(fd, &ch, 1) > 0) {
        if (ch == '\n') {
            
            table[count].offset = line_start;
            table[count].length = current_offset - line_start;
            table[count].idx = count;
            count++;
            
            line_start = current_offset + 1;
        }
        current_offset++;
    }
    
    if (current_offset > line_start) {
        if (count >= capacity) {
            capacity++;
            table = realloc(table, capacity * sizeof(LineInfo));
        }
        table[count].offset = line_start;
        table[count].length = current_offset - line_start;
        table[count].idx = count;
        count++;
    }
    
    *line_count = count;
    return table;
}

void main() {
    signal(SIGALRM, alarm_handler);
    
    int fd = open("test.txt", O_RDONLY);
    int line_count;
    LineInfo *table = build_line_table(fd, &line_count);

    printf("№\tОтступ\tДлина\n");
    for (int i = 0; i < line_count; i++) {
        printf("%d\t%ld\t%d\n", table[i].idx, table[i].offset, table[i].length);
    }

    int line_number;
    char line_buffer[1024];

    printf("\nУ вас есть 5 секунд чтобы ввести номер строки (0 для выхода): \n");
    
    alarm(5);
    
    while(!time_out) {
        if (nonblocking_scanf(&line_number) == 1) {

            alarm(0);
            
            if (line_number == 0) {
                break;
            }
            
            if (line_number < 1 || line_number > line_count) {
                printf("Ошибка: номер строки должен быть от 1 до %d\n", line_count);
                alarm(5);
                continue;
            }
            
            int index = line_number - 1;
            
            lseek(fd, table[index].offset, SEEK_SET);
            
            read(fd, line_buffer, table[index].length);
            line_buffer[table[index].length] = '\0';
            
            printf("Строка %d: %s\n", line_number, line_buffer);
            printf("Введите номер строки (0 для выхода):\n");
            
            alarm(5);
        }
        
    }

    if (time_out) {        
        printf("\nВремя вышло! Вывод всего текста:\n");
        
        for (int i = 0; i < line_count; i++) {
            lseek(fd, table[i].offset, SEEK_SET);
            read(fd, line_buffer, table[i].length);
            line_buffer[table[i].length] = '\0';
            printf("Строка %d: %s\n", i + 1, line_buffer);
        }
    }

    free(table);
    close(fd);
}