#include <stdlib.h>

#include "cexcept.h"

typedef void free_func(void *);

typedef struct _free_node {
    void *ptr;
    free_func *free;
} free_node;

typedef struct _cexcept_free_list {
    size_t capacity;
    size_t length;
    free_node *data;
} cexcept_free_list;

// This function is needed rather than using calloc or memset to 0 because
// NULL is not necessarily 0 on all systems.
static void null_nodes(free_node* nodes, size_t number) {
    free_node *node = nodes;
    for (size_t ii=0; ii<number; ii++) {
        node->ptr = NULL;
        node->free = NULL;
        node++;
    }
}

static const size_t FREE_LIST_INITIAL_SIZE = 4;
cexcept_free_list *cexcept_free_list_new() {
    cexcept_free_list *list = malloc(sizeof(cexcept_free_list));
    if (!list) {
        return NULL;
    }
    list->data = malloc(FREE_LIST_INITIAL_SIZE * sizeof(free_node));
    if (!list->data) {
        free(list);
        return NULL;
    }
    null_nodes(list->data, FREE_LIST_INITIAL_SIZE);
    list->length = 0;
    list->capacity = FREE_LIST_INITIAL_SIZE;
    return list;
}

void cexcept_free(cexcept_free_list *list) {
    for (size_t ii=0; ii<list->capacity; ii++) {
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

cexcept cexcept_free_list_add(cexcept_free_list *list, void *ptr, void *ffunc) {
    // Look for the value, do not add if already inserted
    free_func *ff = (free_func*)ffunc;
    for (size_t ii=0; ii<list->capacity; ii++) {
        if (ptr == list->data[ii].ptr) {
            return CEXCEPT_OK;
        }
    }
    // Expand the amount of memory if capacity is full
    if (list->length >= list->capacity) {
        free_node *new_data = realloc(
            list->data, 
            (list->capacity*2)*sizeof(free_node)
        );
        if (new_data) {
            list->data = new_data;
            null_nodes(&(list->data[list->capacity]), list->capacity);
            list->capacity *= 2;
        } else {
            CEXCEPT_THROW("Could not expand free list memory\n")
        }
    }
    // Insert the value
    bool inserted = false;
    for (size_t ii=0; ii<list->capacity; ii++) {
        if (NULL == list->data[ii].ptr) {
            list->data[ii].ptr = ptr;
            list->data[ii].free = ff;
            inserted = true;
            break;
        }
    }
    if (!inserted) {
        CEXCEPT_THROW("Could not add pointer to free list\n");
    }
    list->length++;
    return CEXCEPT_OK;
}

void cexcept_free_list_remove(cexcept_free_list *list, void *ptr, bool do_free) {
    for (size_t ii=0; ii<list->capacity; ii++) {
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
            list->length--;
            return;
        }
    }
    CEXCEPT_WARN("Address %p was not found in free list\n", ptr);
}
