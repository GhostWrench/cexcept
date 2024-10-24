#include "cexcept.h"

#include <stdlib.h>

typedef struct {
    int *data1;
    int *data2;
} complex_data;

void complex_data_init(complex_data *data) {
    data->data1 = malloc(100);
    data->data2 = malloc(100);
}

void complex_data_free(void *data) {
    complex_data *cd = data;
    if (cd->data1) free(cd->data1);
    if (cd->data2) free(cd->data2);
}

void notbuggy() {
    cexcept_free_list *cexceptfl = cexcept_free_list_new(8);
    int *arr1 = malloc(100);
    cexcept_free_list_add(cexceptfl, arr1, NULL);
    CEXCEPT_INFO("I am a function in a test!\n");
    CEXCEPT_WARN("Only allocated %d array!\n", 1);
    cexcept_free(cexceptfl);
}

cexcept buggy1() {
    cexcept_free_list *cexceptfl = cexcept_free_list_new(8);
    int *arr1 = malloc(100);
    cexcept_free_list_add(cexceptfl, arr1, free);
    int *arr2 = malloc(100);
    cexcept_free_list_add(cexceptfl, arr2, free);
    cexcept_free_list_remove(cexceptfl, arr1, false);
    free(arr1);
    arr1 = malloc(200);
    cexcept_free_list_add(cexceptfl, arr1, free);
    cexcept_free_list_remove(cexceptfl, arr2, true);
    CEXCEPT_THROW_F(cexceptfl, "Something happened ??\n");
}

cexcept buggy2() {
    cexcept_free_list *cexceptfl = cexcept_free_list_new(6);
    complex_data cd1 = {0};
    complex_data_init(&cd1);
    cexcept_free_list_add(cexceptfl, &cd1, complex_data_free);
    complex_data cd2 = {0};
    complex_data_init(&cd2);
    cexcept_free_list_add(cexceptfl, &cd2, complex_data_free);
    complex_data cd3 = {0};
    complex_data_init(&cd3);
    cexcept_free_list_add(cexceptfl, &cd3, complex_data_free);
    int *simple_data1 = malloc(100);
    cexcept_free_list_add(cexceptfl, simple_data1, free);
    int *simple_data2 = malloc(100);
    cexcept_free_list_add(cexceptfl, simple_data2, free);
    int *simple_data3 = malloc(100);
    cexcept_free_list_add(cexceptfl, simple_data3, free);
    int *simple_data4 = malloc(100);
    // Try to add more than possible
    CEXCEPT_CHECK_ADD(cexceptfl, simple_data4, free);
    cexcept_free(cexceptfl);
    return CEXCEPT_OK;
}

int main() {
    int retval = EXIT_SUCCESS;
    notbuggy();
    if (buggy1()) {
        fprintf(stderr, "buggy2 did not throw an error\n");
        retval = EXIT_FAILURE;
    }
    if (buggy2()) {
        fprintf(stderr, "buggy3 did not throw an error\n");
        retval = EXIT_FAILURE;
    }
    return retval;
}