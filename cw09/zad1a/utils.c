#include "utils.h"

/**
 * fixed_array_list structure is designed for dealing only with unsigned values
 * because there is ambiguity related to -1
 *
 * I didn't do that on pointers because of simplicity. This solution fit for the purpose.
 * */
// array list
fixed_array_list *fal_init(int list_size) {
    fixed_array_list *l = calloc(1, sizeof(fixed_array_list));
    l->data = calloc((size_t) list_size, sizeof(LIST_TYPE));
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

int fal_add(fixed_array_list *l, LIST_TYPE item) {
    if (fal_is_full(l) == 1) {
        return LIST_ERR;
    }

    l->data[l->items_num++] = item;

    return LIST_OK;
}

LIST_TYPE fal_pop(fixed_array_list *l) {
    return fal_remove(l, 0);
}

LIST_TYPE fal_get(fixed_array_list *l) {
    if (fal_empty(l) == 1) {
        return LIST_ERR;
    }

    return l->data[0];
}

LIST_TYPE fal_get_n(fixed_array_list *l, int index) {
    if (fal_empty(l) == 1 || index >= l->items_num || index < 0) {
        return LIST_ERR;
    }

    return l->data[index];
}

void fal_print(fixed_array_list *l) {
    printf(ANSI_COLOR_MAGENTA "fixed_array_list print:" ANSI_COLOR_RESET "\n");
    for (int i=0; i<l->items_num; i++) {
        printf(ANSI_COLOR_MAGENTA "\t%d) value = %lld" ANSI_COLOR_RESET "\n", i+1, fal_get_n(l, i));
    }
    printf(ANSI_COLOR_MAGENTA "\tlist max size: %d" ANSI_COLOR_RESET "\n", l->size);
}

LIST_TYPE fal_remove(fixed_array_list *l, int index) {
    if (fal_empty(l) == 1 || index >= l->items_num || index < 0) {
        return LIST_ERR;
    }

    LIST_TYPE item = l->data[index];
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