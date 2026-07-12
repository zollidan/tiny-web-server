#include "counter.h"
#include <stdio.h>

int next_visitor_number() {
    int count = 0;

    FILE *fp = fopen(COUNTER_FILE, "r");
    if (fp != NULL) {
        fscanf(fp, "%d", &count);
        fclose(fp);
    }
    count++;
    fp = fopen(COUNTER_FILE, "w");
    if (fp != NULL) {
        fprintf(fp, "%d", count);
        fclose(fp);
    }
    return count;
}