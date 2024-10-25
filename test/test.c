#include "cexcept.h"

#include <stdlib.h>

typedef struct {
    int *data1;
    int *data2;
} complex_data;

void complex_data_init(complex_data *data) {
    data->data1 = malloc(100*sizeof(int));
    data->data2 = malloc(100*sizeof(int));
}

void complex_data_free(complex_data *cd) {
    if (cd->data1) free(cd->data1);
    if (cd->data2) free(cd->data2);
}

cexcept bad_func() {
    CEXCEPT_THROW("You called 'bad_func'... why?\n");
}

void notbuggy() {
    cexcept_free_list *cfl = cexcept_free_list_new();
    int *arr1 = malloc(100*sizeof(int));
    cexcept_free_list_add(cfl, arr1, NULL);
    CEXCEPT_INFO("I am a function in a test!\n");
    CEXCEPT_WARN("Only allocated %d array!\n", 1);
    cexcept_free(cfl);
}

cexcept buggy1() {
    cexcept_free_list *cfl = cexcept_free_list_new();
    int *arr1 = malloc(100*sizeof(int));
    cexcept_free_list_add(cfl, arr1, free);
    int *arr2 = malloc(100*sizeof(int));
    cexcept_free_list_add(cfl, arr2, free);
    cexcept_free_list_remove(cfl, arr1, false);
    free(arr1);
    arr1 = malloc(100*sizeof(int));
    cexcept_free_list_add(cfl, arr1, free);
    cexcept_free_list_remove(cfl, arr2, true);
    CEXCEPT_THROW_F(cfl, "Something happened ??\n");
}

cexcept buggy2() {
    cexcept_free_list *cfl = cexcept_free_list_new();
    complex_data cd1 = {0};
    complex_data_init(&cd1);
    CEXCEPT_CHECK_ADD(cfl, &cd1, complex_data_free);
    complex_data cd2 = {0};
    complex_data_init(&cd2);
    CEXCEPT_CHECK_ADD(cfl, &cd2, complex_data_free);
    complex_data cd3 = {0};
    complex_data_init(&cd3);
    CEXCEPT_CHECK_ADD(cfl, &cd3, complex_data_free);
    int *sd1 = malloc(100*sizeof(int));
    if (!sd1) CEXCEPT_THROW_F(cfl, "Could not allocate memory!\n");
    sd1[0] = 1;
    CEXCEPT_CHECK_ADD(cfl, sd1, free);
    int *sd2 = malloc(100*sizeof(int));
    if (!sd2) CEXCEPT_THROW_F(cfl, "Could not allocate memory!\n");
    sd2[0] = 2;
    CEXCEPT_CHECK_ADD(cfl, sd2, free);
    int *sd3 = malloc(100*sizeof(int));
    if (!sd3) CEXCEPT_THROW_F(cfl, "Could not allocate memory!\n");
    sd3[0] = 3;
    CEXCEPT_CHECK_ADD(cfl, sd3, free);
    int *sd4 = malloc(100*sizeof(int));
    if (!sd4) CEXCEPT_THROW_F(cfl, "Could not allocate memory!\n");
    sd4[0] = 4;
    CEXCEPT_CHECK_ADD(cfl, sd4, free);
    sd4[1] = 8;
    // Remove some items
    cexcept_free_list_remove(cfl, &cd2, true);
    cexcept_free_list_remove(cfl, sd3, true);
    // Move some stuff
    cexcept_free_list_remove(cfl, sd4, false);
    sd4[50] = 50;
    CEXCEPT_CHECK_ADD(cfl, sd4, free);
    CEXCEPT_CHECK_F(cfl, bad_func());
    cexcept_free(cfl);
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