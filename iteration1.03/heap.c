#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "heap.h"

struct heap_node
{
    heap_node_t *next;
    heap_node_t *prev;
    heap_node_t *parent;
    heap_node_t *child;
    uint32_t degree;
    uint32_t mark;
    void *data;
};

void heap_init(heap_t *h, int32_t (*compare)(const void *key, const void *with), void (*delete_data)(void *))
{
    h->min = NULL;
    h->size = 0;
    h->datum_delete = delete_data;
    h->compare = compare;
}
heap_node_t *heap_insert(heap_t *h, void *v)
{
    heap_node_t *n;
    assert((n = calloc(1, sizeof(*n))));
    n->data = v;
    if (h->min)
    {
        n->next = h->min;
        n->prev = h->min->prev;
        n->prev->next = n;
        h->min->prev = n;
    }
    else
    {
        n->next = n->prev = n; // nothing in heap so everything is n
    }
    if(!h->min || (h->compare(v, h->min->data) < 0)){
        h->min = n;
    }
    h->size++;
    return n;
}


static void heap_consolidate(heap_t *h)
{
    uint32_t i;
    heap_node_t *x, *y, *n;
    heap_node_t *a[64]; /* Need ceil(lg(h->size)), so this is good  *
                         * to the limit of a 64-bit address space,  *
                         * and much faster than any lg calculation. */

    memset(a, 0, sizeof(a));

    h->min->prev->next = NULL;

    for (x = n = h->min; n; x = n)
    {
        n = n->next;

        while (a[x->degree])
        {
            y = a[x->degree];
            if (h->compare(x->data, y->data) > 0)
            {
                swap(x, y);
            }
            a[x->degree] = NULL;
            heap_link(h, y, x);
        }
        a[x->degree] = x;
    }

    for (h->min = NULL, i = 0; i < 64; i++)
    {
        if (a[i])
        {
            if (h->min)
            {
                insert_heap_node_in_list(a[i], h->min);
                if (h->compare(a[i]->data, h->min->data) < 0)
                {
                    h->min = a[i];
                }
            }
            else
            {
                h->min = a[i];
                a[i]->next = a[i]->prev = a[i];
            }
        }
    }
}

void *heap_remove_min(heap_t *h){
    void *v;
    heap_node_t *n;
    v = NULL;
    if(h->min){
        v = h->min->data;
        if(h->size == 1){
            free(h->min);
            h->min = NULL;
        }
        else{
            if((n = h->min->child)){
                for(;n->parent; n = n->next){
                    n->parent = NULL;
                }
            }
            if(h->min && h->min->child){
                h->min->next->prev = h->min->child->prev; 
                h->min->child->prev->next = h->min->next;
                h->min->next = h->min->child;
                h->min->child->prev = h->min;   //?
            }
            n = h->min;
            n->next->prev = n->prev; //remove it
            n->prev->next = n->next;
            h->min = n->next;
            free(n);
            heap_consolidate(h);
        }
        h->size--;
    }
    return v;
}