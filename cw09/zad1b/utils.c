#include "utils.h"

// array list
fixed_array_list *fal_init(int list_size) {
    fixed_array_list *l = calloc(1, sizeof(fixed_array_list));
    l->data = calloc((size_t) list_size, sizeof(int));
    l->size = list_size;
    l->items_num = 0;

    return l;
}

void fal_destroy(fixed_array_list *l) {
    free(l->data);
    free(l);
}

int fal_len(fixed_array_list *l) {
    return l->items_num;
}

int fal_empty(fixed_array_list *l) {
    return (l->items_num == 0) ? 1 : 0;
}

int fal_is_full(fixed_array_list *l) {
    return (l->items_num == l->size) ? 1 : 0;
}

int fal_add(fixed_array_list *l, int item) {
    if (fal_is_full(l) == 1) {
        return LIST_ERR;
    }

    l->data[l->items_num++] = item;

    return LIST_OK;
}
//
int fal_pop(fixed_array_list *l) {
    return fal_remove(l, 0);
}

int fal_remove(fixed_array_list *l, int index) {
    if (fal_empty(l) == 1 || index >= l->items_num || index < 0) {
        return LIST_ERR;
    }

    int item = l->data[index];
    for (int i=index; i<l->items_num-1; i++) {
        l->data[i] = l->data[i+1];
    }

    --l->items_num;
    return item;
}

// time
// returns time in microseconds
__time_t get_time() {
    struct timespec time;
    if (clock_gettime(CLOCK_MONOTONIC, &time) == -1) {
        printf("clock_gettime error\n");
        exit(EXIT_FAILURE);
    }

    return time.tv_nsec / 1000;
}

// returns time in nanoseconds
__time_t get_ntime() {
    struct timespec time;
    if (clock_gettime(CLOCK_MONOTONIC, &time) == -1) {
        printf("clock_gettime error\n");
        exit(EXIT_FAILURE);
    }

    return time.tv_nsec;
}