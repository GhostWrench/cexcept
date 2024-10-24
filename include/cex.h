/**
 * cex is a minimal C exception handling library for C that introduces a few 
 * small macros for writing messages to stderr, cleaning up allocated memory
 * and exiting from functions when exceptional conditions occur. Functions 
 * which throw errors must have the 'cex' return type so there is no 
 * confusion about which functions in your code may or may not throw 
 * said exceptions
 */

#ifndef _CEX_H
#define _CEX_H

#include <stdio.h>
#include <stdbool.h>

// Versioning
#define CEX_VERSION_MAJOR 0
#define CEX_VERSION_MINOR 1
#define CEX_VERSION_PATCH 0

// Setup verbosity level
#define CEX_VERBOSITY_NONE 0
#define CEX_VERBOSITY_FAILURE 1
#define CEX_VERBOSITY_WARNING 2
#define CEX_VERBOSITY_INFO 3
#ifndef CEX_VERBOSITY
#define CEX_VERBOSITY CEX_VERBOSITY_WARNING
#endif

/**
 * Exception type, this must be the return type of any function that uses 
 * CEX_THROW, CEX_THROW_F, CEX_CHECK or CEX_CHECK_F macros
 */
typedef enum _cex {
    CEX_FAILURE,
    CEX_OK
} cex;

/**
 * Throw an exception that is printed to stderr and then return from the 
 * calling function.
 */
#define CEX_THROW(...) { \
    if (CEX_VERBOSITY >= CEX_VERBOSITY_FAILURE) { \
        fprintf(stderr, "File: %s, Line: %d, Exception:\n", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
    } \
    return CEX_FAILURE; \
}

/**
 * Output a warning to stderr to indicate an unusual or unexpected condition
 * but do not return from the function
 */
#define CEX_WARN(...) { \
    if (CEX_VERBOSITY >= CEX_VERBOSITY_WARNING) { \
        fprintf(stderr, "File: %s, Line: %d, Warning:\n", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
    } \
}

/**
 * Output information to stderr that may be useful for debugging purposes
 */
#define CEX_INFO(...) { \
    if (CEX_VERBOSITY >= CEX_VERBOSITY_INFO) { \
        fprintf(stderr, "File: %s, Line: %d, Info:\n", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
    } \
}

/**
 * Check if an expression (usually a function) returns a CEX_FAILURE, if it does
 * return from the calling function with CEX_FAILURE
 */
#define CEX_CHECK(expr) { \
    cex ex = (expr); \
    if (ex == CEX_FAILURE) { \
        return ex; \
    } \
}

/**
 * Type for a function that free's memory (like 'free'). Used to allow a
 * cex free list to free more complex data types if needed
 */
typedef void cex_free_func(void *);

/**
 * List containing pointers to memory location along with a free function so
 * that memory can be automatically free'd by the CEX_THROW_F and CEX_CHECK_F
 * macros
 */
typedef struct _cex_free_list cex_free_list;

/**
 * Allocate memory for a free list with capacity for 'length' number of heap
 * pointers to free
 */
cex_free_list *cex_free_list_new(size_t length);

/**
 * Add a heap address and free function to the list of things that need to be
 * free'd. This will only allow addition of unique values. Will throw an 
 * exception if attempting to add more than the length of the list.
 */
cex cex_free_list_add(cex_free_list *list, void *ptr, cex_free_func *free);

/**
 * Remove a pointer from the free list. If the 'do_free' parameter is true then
 * it will also free the pointer using the supplied function. Otherwise, the 
 * data will not be free'd and can be moved.
 */
void cex_free_list_remove(cex_free_list *list, void *ptr, bool do_free);

/**
 * Free all items in the cex_free_list
 */
void cex_free(cex_free_list *list);

/**
 * Free all the allocated heap memory in the cex_free_list, throw an
 * exception that is printed to stderr and then return from the calling function
 * with CEX_FAILURE
 */
#define CEX_THROW_F(free_list, ...) { \
    cex_free(free_list); \
    CEX_THROW(__VA_ARGS__); \
}

/**
 * Check if an expression (usually a function) returns a CEX_FAILURE, if it does
 * free all heap memory allocated in the provided 'cex_free_list' and then 
 * return from the calling function with CEX_FAILURE.
 */
#define CEX_CHECK_F(free_list, expr) { \
    cex ex = (expr); \
    if (ex == CEX_FAILURE) { \
        cex_free(free_list); \
        return ex; \
    } \
}

/**
 * This macro is similar to the cex_free_list_add function except it frees the
 * value being added upon failure along with the rest of the list and then
 * returns from the calling function with CEX_FAILURE.
 */
#define CEX_CHECK_ADD(list, ptr, func) { \
    if (!cex_free_list_add((list), (ptr), (func))) { \
        (func)((ptr)); \
        cex_free(list); \
        return CEX_FAILURE; \
    } \
}

#endif // _CEX_H
