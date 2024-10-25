#include <stdlib.h>
#include <stdio.h>

#include "cexcept.h"

cexcept real_main() {
    
    // Setup memory handling
    cexcept_free_list *cfl = cexcept_free_list_new();
    
    // Allocate memory to read numbers from a file
    long *numbers = calloc(6, sizeof(long));
    if (!numbers) {
        CEXCEPT_THROW_F(cfl, "Could not allocate memory for numbers\n");
    }
    cexcept_free_list_add(cfl, numbers, NULL);

    // Open the file
    const char *file_name = "./test/data/numbers.txt";
    FILE *file = fopen(file_name, "r");
    if (!file) {
        CEXCEPT_THROW_F(cfl, "Could not open file %s\n", file_name);
    }
    cexcept_free_list_add(cfl, file, fclose);

    // Read each line from the file and put it in 'numbers'
    char line[256] = {0};
    size_t line_idx = 0;
    char current_char = '\0';
    size_t numbers_idx = 0;
    while ((current_char = fgetc(file)) != EOF) {
        if (current_char != '\n') {
            line[line_idx] = current_char;
            line_idx ++;
        } else {
            line[line_idx] = '\0';
            line_idx = 0;
            numbers[numbers_idx] = strtol(line, NULL, 10);
            if (0 == numbers[numbers_idx]) {
                CEXCEPT_THROW_F(cfl, "%s is not a number\n", line);
            }
            numbers_idx++;
        }
    }
    cexcept_free(cfl);
    return CEXCEPT_OK;
}

int main () {
    if (real_main()) {
        // It SHOULD fail
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}