#include <stdlib.h>

#include "cexcept.h"

typedef struct _cexcept_free_node {
    void *ptr;
    cexcept_free_func *free;
} cexcept_free_node;

typedef struct _cexcept_free_list {
    size_t length;
    cexcept_free_node *data;
} cexcept_free_list;

cexcept_free_list *cexcept_free_list_new(size_t length) {
    cexcept_free_list *list = calloc(1, sizeof(cexcept_free_list));
    if (!list) {
        return NULL;
    }
    list->data = calloc(length, sizeof(cexcept_free_node));
    if (!list->data) {
        free(list);
        return NULL;
    }
    list->length = length;
    return list;
}

void cexcept_free(cexcept_free_list *list) {
    for (size_t ii=0; ii<list->length; ii++) {
        if (list->data[ii].ptr) {
            if (list->data[ii].free) {
                list->data[ii].free(list->data[ii].ptr);
            } else {
                free(list->data[ii].ptr);
            }
            list->data[ii].ptr = NULL;
            list->data[ii].free = NULL;
        }
    }
    free(list->data);
    free(list);
}

cexcept cexcept_free_list_add(cexcept_free_list *list, void *ptr, cexcept_free_func *free) {
    // Look for the value, do not add if already inserted
    for (size_t ii=0; ii<list->length; ii++) {
        if (ptr == list->data[ii].ptr) {
            return CEXCEPT_OK;
        }
    }
    // Insert the value
    bool inserted = false;
    for (size_t ii=0; ii<list->length; ii++) {
        if (NULL == list->data[ii].ptr) {
            list->data[ii].ptr = ptr;
            list->data[ii].free = free;
            inserted = true;
            break;
        }
    }
    if (!inserted) {
        CEXCEPT_THROW("cexcept_free_list full, cannot add more pointers\n");
    }
    return CEXCEPT_OK;
}

void cexcept_free_list_remove(cexcept_free_list *list, void *ptr, bool do_free) {
    for (size_t ii=0; ii<list->length; ii++) {
        if (ptr == list->data[ii].ptr) {
            if (do_free) {
                if (list->data[ii].free) {
                    list->data[ii].free(list->data[ii].ptr);
                } else {
                    free(list->data[ii].ptr);
                }
            }
            list->data[ii].ptr = NULL;
            list->data[ii].free = NULL;
            return;
        }
    }
}
