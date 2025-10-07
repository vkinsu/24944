#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


typedef struct {
    long offset;
    int length;
    int idx;
} LineInfo;

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
    int fd = open("test.txt", O_RDONLY);
    int line_count;
    LineInfo *table = build_line_table(fd, &line_count);

    printf("№\tОтступ\tДлина\n");
    for (int i = 0; i < line_count; i++) {
        printf("%d\t%ld\t%d\n", table[i].idx + 1, table[i].offset, table[i].length);
    }

    int line_number;
    char line_buffer[1024];
    
    while (1) {
        printf("\nВведите номер строки (0 для выхода): ");
        scanf("%d", &line_number);
        
        if (line_number == 0) {
            break;
        }
        
        if (line_number < 1 || line_number > line_count) {
            printf("Ошибка: номер строки должен быть от 1 до %d\n", line_count);
            continue;
        }
        
        int index = line_number - 1;
        
        lseek(fd, table[index].offset, SEEK_SET);
        
        read(fd, line_buffer, table[index].length);
        line_buffer[table[index].length] = '\0';
        
        printf("Строка %d: %s\n", line_number, line_buffer);
    }

    free(table);
    close(fd);
}