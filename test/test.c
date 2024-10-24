#include "cex.h"

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
    cex_free_list *cexfl = cex_free_list_new(8);
    int *arr1 = malloc(100);
    cex_free_list_add(cexfl, arr1, NULL);
    CEX_INFO("I am a function in a test!\n");
    CEX_WARN("Only allocated %d array!\n", 1);
    cex_free(cexfl);
}

cex buggy1() {
    cex_free_list *cexfl = cex_free_list_new(8);
    int *arr1 = malloc(100);
    cex_free_list_add(cexfl, arr1, free);
    int *arr2 = malloc(100);
    cex_free_list_add(cexfl, arr2, free);
    cex_free_list_remove(cexfl, arr1, false);
    free(arr1);
    arr1 = malloc(200);
    cex_free_list_add(cexfl, arr1, free);
    cex_free_list_remove(cexfl, arr2, true);
    if (2 == 2)
        CEX_THROW_F(cexfl, "Something happened ??\n");
}

cex buggy2() {
    cex_free_list *cexfl = cex_free_list_new(6);
    complex_data cd1 = {0};
    complex_data_init(&cd1);
    cex_free_list_add(cexfl, &cd1, complex_data_free);
    complex_data cd2 = {0};
    complex_data_init(&cd2);
    cex_free_list_add(cexfl, &cd2, complex_data_free);
    complex_data cd3 = {0};
    complex_data_init(&cd3);
    cex_free_list_add(cexfl, &cd3, complex_data_free);
    int *simple_data1 = malloc(100);
    cex_free_list_add(cexfl, simple_data1, free);
    int *simple_data2 = malloc(100);
    cex_free_list_add(cexfl, simple_data2, free);
    int *simple_data3 = malloc(100);
    cex_free_list_add(cexfl, simple_data3, free);
    int *simple_data4 = malloc(100);
    // Try to add more than possible
    CEX_CHECK_ADD(cexfl, simple_data4, free);
    cex_free(cexfl);
    return CEX_OK;
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