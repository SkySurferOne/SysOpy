#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LIST_SIZE 10

/*
 * Program returns random number of numbers from range [0; LIST_SIZE] without repetition.
 * */
int main() {
    fixed_array_list *list = fal_init(LIST_SIZE);

    for (int i=0; i<list->size; i++) {
        fal_add(list, i);
    }

    srand(time(NULL));
    int mod_num = (rand() % (LIST_SIZE-1)) + 1;
    printf("Modification number %d\n", mod_num);

    for (int i=0; i<mod_num; i++) {
        int index = rand() % fal_len(list);
        int item = fal_remove(list, index);
        printf("%d) item = %d\n", i+1, item);
    }

    fal_destroy(list);
    return 0;
}
