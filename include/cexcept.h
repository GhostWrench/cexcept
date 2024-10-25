/**
 * cexcept is a minimal C exception handling library for C that introduces a few 
 * small macros for writing messages to stderr, cleaning up allocated memory
 * and exiting from functions when exceptional conditions occur. Functions 
 * which throw errors must have the 'cexceptcept' return type so there is no 
 * confusion about which functions in your code may or may not throw 
 * said exceptions
 */

#ifndef _CEXCEPT_H
#define _CEXCEPT_H

#include <stdio.h>
#include <stdbool.h>

// Versioning
#define CEXCEPT_VERSION_MAJOR 0
#define CEXCEPT_VERSION_MINOR 1
#define CEXCEPT_VERSION_PATCH 0

// Setup verbosity level
#define CEXCEPT_VERBOSITY_NONE 0
#define CEXCEPT_VERBOSITY_FAILURE 1
#define CEXCEPT_VERBOSITY_WARNING 2
#define CEXCEPT_VERBOSITY_INFO 3
#ifndef CEXCEPT_VERBOSITY
#define CEXCEPT_VERBOSITY CEXCEPT_VERBOSITY_WARNING
#endif

/**
 * Exception type, this must be the return type of any function that uses
 * CEXCEPT_THROW, CEXCEPT_THROW_F, CEXCEPT_CHECK or CEXCEPT_CHECK_F macros
 */
typedef enum _cexcept {
    CEXCEPT_FAILURE,
    CEXCEPT_OK
} cexcept;

/**
 * Throw an exception that is printed to stderr and then return from the 
 * calling function.
 */
#define CEXCEPT_THROW(...) { \
    if (CEXCEPT_VERBOSITY >= CEXCEPT_VERBOSITY_FAILURE) { \
        fprintf(stderr, "File: %s, Line: %d, Exception:\n", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
    } \
    return CEXCEPT_FAILURE; \
}

/**
 * Output a warning to stderr to indicate an unusual or unexpected condition
 * but do not return from the function
 */
#define CEXCEPT_WARN(...) { \
    if (CEXCEPT_VERBOSITY >= CEXCEPT_VERBOSITY_WARNING) { \
        fprintf(stderr, "File: %s, Line: %d, Warning:\n", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
    } \
}

/**
 * Output information to stderr that may be useful for debugging purposes
 */
#define CEXCEPT_INFO(...) { \
    if (CEXCEPT_VERBOSITY >= CEXCEPT_VERBOSITY_INFO) { \
        fprintf(stderr, "File: %s, Line: %d, Info:\n", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
    } \
}

/**
 * Check if an expression (usually a function) returns a CEXCEPT_FAILURE, if it
 * does return from the calling function with CEXCEPT_FAILURE
 */
#define CEXCEPT_CHECK(expr) { \
    cexcept ex = (expr); \
    if (ex == CEXCEPT_FAILURE) { \
        return ex; \
    } \
}

/**
 * Type for a function that free's memory (like 'free'). Used to allow a
 * cexcept free list to free more complex data types if needed
 */
typedef void cexcept_free_func(void *);

/**
 * List containing pointers to memory location along with a free function so
 * that memory can be automatically free'd by the CEXCEPT_THROW_F and
 * CEXCEPT_CHECK_F macros
 */
typedef struct _cexcept_free_list cexcept_free_list;

/**
 * Allocate memory for a free list to track memory allocated on the stack
 */
cexcept_free_list *cexcept_free_list_new();

/**
 * Add a heap address and free function to the list of things that need to be
 * free'd. This will only allow addition of unique values. Will throw an 
 * exception if attempting to add more than the length of the list.
 */
cexcept _cexcept_free_list_add(cexcept_free_list *list, void *ptr, cexcept_free_func *free);

/**
 * Convienience macro for _cexcept_free_list_add so that the user can give it
 * any kind function pointer rather than just a 'void func(void *)' function.
 */
#define cexcept_free_list_add(list, ptr, free) _cexcept_free_list_add((list), (ptr), (cexcept_free_func*)(free))

/**
 * Remove a pointer from the free list. If the 'do_free' parameter is true then
 * it will also free the pointer using the supplied function. Otherwise, the 
 * data will not be free'd and can be moved.
 */
void cexcept_free_list_remove(cexcept_free_list *list, void *ptr, bool do_free);

/**
 * Free all items in the cexcept_free_list
 */
void cexcept_free(cexcept_free_list *list);

/**
 * Free all the allocated heap memory in the cexcept_free_list, throw an
 * exception that is printed to stderr and then return from the calling function
 * with CEXCEPT_FAILURE
 */
#define CEXCEPT_THROW_F(free_list, ...) { \
    cexcept_free(free_list); \
    CEXCEPT_THROW(__VA_ARGS__); \
}

/**
 * Check if an expression (usually a function) returns a CEXCEPT_FAILURE, if it does
 * free all heap memory allocated in the provided 'cexcept_free_list' and then 
 * return from the calling function with CEXCEPT_FAILURE.
 */
#define CEXCEPT_CHECK_F(free_list, expr) { \
    cexcept ex = (expr); \
    if (ex == CEXCEPT_FAILURE) { \
        cexcept_free(free_list); \
        return ex; \
    } \
}

/**
 * This macro is similar to the cexcept_free_list_add function except it frees the
 * value being added upon failure along with the rest of the list and then
 * returns from the calling function with CEXCEPT_FAILURE.
 */
#define CEXCEPT_CHECK_ADD(list, ptr, func) { \
    if (!cexcept_free_list_add((list), (ptr), (func))) { \
        _Pragma( "GCC diagnostic push" ) \
        _Pragma( "GCC diagnostic ignored \"-Waddress\"" ) \
        if (ptr) { \
            if (func) { \
                (func)((ptr)); \
            } else { \
                free(ptr); \
            } \
        } \
        _Pragma( "GCC diagnostic pop" ) \
        cexcept_free(list); \
        return CEXCEPT_FAILURE; \
    } \
}

#endif // _CEXCEPT_H
