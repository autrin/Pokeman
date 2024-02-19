#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "heap.h"

typdef struct heap_node
{
    heap_node_t *next;
    heap_node_t *prev;
    heap_node_t *parent;
    heap_node_t *child;
    // might need other fields like degree
    void *data;
} heap_node_t;

void heap_init(heap_t *h, int32_t (*compare)(const void *key, const void *with), void (*delete_data)(void *)){
    h->min = NULL;
    h->size = 0;
    h->datum_delete = delete_data;
    h->compare = compare;
}