#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

void fatal(const char msg[], const char file[]) {
    fprintf(stderr, "\n[FATAL ERROR] в файле %s: %s\n", file, msg);
    
    fprintf(stderr, "Причина ОС: %s (код %d)\n", strerror(errno), errno);
    
    fprintf(stderr, "Завершение работы программы.\n\n");
    
    exit(1); 
}