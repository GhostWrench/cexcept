#include <stdlib.h>

#include "cex.h"

typedef struct _cex_free_node {
    void *ptr;
    cex_free_func *free;
} cex_free_node;

typedef struct _cex_free_list {
    size_t length;
    cex_free_node *data;
} cex_free_list;

cex_free_list *cex_free_list_new(size_t length) {
    cex_free_list *list = calloc(1, sizeof(cex_free_list));
    if (!list) {
        return NULL;
    }
    list->data = calloc(length, sizeof(cex_free_node));
    if (!list->data) {
        free(list);
        return NULL;
    }
    list->length = length;
    return list;
}

void cex_free(cex_free_list *list) {
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

cex cex_free_list_add(cex_free_list *list, void *ptr, cex_free_func *free) {
    // Look for the value, do not add if already inserted
    for (size_t ii=0; ii<list->length; ii++) {
        if (ptr == list->data[ii].ptr) {
            return CEX_OK;
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
        CEX_THROW("cex_free_list full, cannot add more pointers\n");
    }
    return CEX_OK;
}

void cex_free_list_remove(cex_free_list *list, void *ptr, bool do_free) {
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
