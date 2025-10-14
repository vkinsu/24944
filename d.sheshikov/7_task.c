#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

volatile sig_atomic_t time_out = 0;

char *file_data = NULL;
size_t file_size = 0;
long *line_offsets = NULL;
int line_count = 0;

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

int get_len(int line_num) {

    int idx = line_num - 1;
    long start = line_offsets[idx];
    long end;
    
    if (idx < line_count - 1) {
        end = line_offsets[idx + 1] - 1;
    } else {
        end = file_size;
    }
    
    int length = end - start;
    return length;
}

void print_line(int line_num) {
    if (line_num < 1 || line_num > line_count) {
        printf("Ошибка: номер строки должен быть от 1 до %d\n", line_count);  
        return;
    }

    long start = line_offsets[line_num  -1];

     printf("Строка %d: %.*s\n", line_num, get_len(line_num), file_data + start);
}

void build_line_table(int fd) {
    struct stat sb;

    fstat(fd, &sb);
    file_size = sb.st_size;
    file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

    int capacity = 100;
    line_offsets = malloc(capacity * sizeof(long));
    line_count = 0;

    line_offsets[line_count++] = 0;

    for(size_t i = 0; i < file_size; i++) {
        if(file_data[i] == '\n') {
            if(line_count >= capacity) {
                capacity *= 2;
                line_offsets = realloc(line_offsets, capacity * sizeof(long));
            }
            if (i + 1 < file_size) {
                line_offsets[line_count++] = i + 1;
            }
        }
    }
}

void print_all_lines() {
    for (int i = 1; i <= line_count; i++) {
        print_line(i);
    }
}

void main() {
    signal(SIGALRM, alarm_handler);
    
    int fd = open("test.txt", O_RDONLY);
    build_line_table(fd);


    printf("№\tОтступ\tДлина\n");
    for (int i = 0; i < line_count; i++) {
        printf("%d\t%ld\t%d\n", i + 1, line_offsets[i], get_len(i + 1));
    }
    int line_number;

    printf("\nУ вас есть 5 секунд чтобы ввести номер строки (0 для выхода): \n");
    
    alarm(5);
    
    while(!time_out) {
        if (nonblocking_scanf(&line_number) == 1) {
            alarm(0);
            
            if (line_number == 0) {
                break;
            }
            
            print_line(line_number);
            printf("Введите номер строки (0 для выхода):\n");
            
            alarm(5);
        }
    }

    if (time_out) {        
        printf("\nВремя вышло! Вывод всего текста:\n");
        print_all_lines();
    }

    munmap(file_data, file_size);
    free(line_offsets);
    close(fd);
}