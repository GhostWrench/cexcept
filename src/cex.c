#include <stdlib.h>

#include "cex.h"

typedef struct _cex_free_list {
    struct {
        void *ptr;
        cex_free_func *free;
    } data[CEX_FREE_LIST_LENGTH];
} cex_free_list;

cex cex_free_list_add(cex_free_list *list, void *ptr, cex_free_func *free) {
    // Look for the value, do not add if already inserted
    for (int ii=0; ii<CEX_FREE_LIST_LENGTH; ii++) {
        if (ptr == list->data[ii].ptr) {
            return CEX_OK;
        }
    }
    // Insert the value
    bool inserted = false;
    for (int ii=0; ii<CEX_FREE_LIST_LENGTH; ii++) {
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
    for (int ii=0; ii<CEX_FREE_LIST_LENGTH; ii++) {
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

void cex_free(cex_free_list *list) {
    for (int ii=0; ii<CEX_FREE_LIST_LENGTH; ii++) {
        if (list->data[ii].ptr) {
            if (list->data[ii].free) {
                list->data[ii].free(list->data[ii].ptr);
            } else {
                free(list->data[ii].ptr);
            }
        }
    }
}
