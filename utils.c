

#include <stdio.h>
#include "inflection.h"
#include "utils.h"

void print_app_comm_data(struct app_comm_msg* comm_data) {
    printf("Size = %d | Action = %d | Data = ", comm_data->header.size, comm_data->header.action);
    int i = 0;
    for (; i < comm_data->header.size; i++) {
        putc(comm_data->data[i], stdout);
    }
    putc('\n', stdout);
}