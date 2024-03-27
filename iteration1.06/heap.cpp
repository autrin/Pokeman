#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "heap.h"

struct heap_node
{
    heap_node_t* next;
    heap_node_t* prev;
    heap_node_t* parent;
    heap_node_t* child;
    uint32_t degree;
    uint32_t mark;
    void* data;
};

#define swap(a, b) ({     \
    typeof(a) _tmp = (a); \
    (a) = (b);            \
    (b) = _tmp;           \
})

void heap_init(heap_t* h, int32_t(*compare)(const void* key, const void* with), void (*delete_data)(void*))
{
    h->min = NULL;
    h->size = 0;
    h->datum_delete = delete_data;
    h->compare = compare;
}
heap_node_t* heap_insert(heap_t* h, void* v)
{
    heap_node_t* n;
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
    if (!h->min || (h->compare(v, h->min->data) < 0)) {
        h->min = n;
    }
    h->size++;
    return n;
}

static void heap_link(heap_t* h, heap_node_t* node, heap_node_t* root)
{
    /*  remove_heap_node_from_list(node)*/
    if (root->child)
    {
        // Now insert the node
        node->next = root->child;
        node->prev = root->child->prev;
        node->prev->next = node;
        root->child->prev = node;

    }
    else
    {
        root->child = node;
        node->next = node->prev = node;
    }
    node->parent = root;
    root->degree++;
    node->mark = 0;
}

static void heap_consolidate(heap_t* h)
{
    uint32_t i;
    heap_node_t* x, * y, * n;
    heap_node_t* a[64]; /* Need ceil(lg(h->size)), so this is good  *
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
                swap(x, y); // x should be smaller, so swap it with its next one, which is bigger
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
                // Now insert the node
                a[i]->next = h->min;
                a[i]->prev = h->min->prev;
                a[i]->prev->next = a[i];
                h->min->prev = a[i];
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

void* heap_remove_min(heap_t* h) {
    void* v;
    heap_node_t* n;
    v = NULL;
    if (h->min) {
        v = h->min->data;
        if (h->size == 1) {
            free(h->min);
            h->min = NULL;
        }
        else {
            if ((n = h->min->child)) {
                for (;n->parent; n = n->next) {
                    n->parent = NULL;
                }
            }
            if (h->min && h->min->child) {
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

static void heap_cut(heap_t* h, heap_node_t* n, heap_node_t* p)
{
    if (!--p->degree)
    {
        p->child = NULL;
    }
    if (p->child == n)
    {
        p->child = p->child->next;
    }

    // remove the node
    n->next->prev = n->prev;
    n->prev->next = n->next;

    n->parent = NULL;
    n->mark = 0;

    // Now insert the node
    n->next = h->min;
    n->prev = h->min->prev;
    n->prev->next = n;
    h->min->prev = n;
}

static void heap_cascading_cut(heap_t* h, heap_node_t* n)
{
    heap_node_t* p;

    if ((p = n->parent))
    {
        if (!n->mark)
        {
            n->mark = 1;
        }
        else
        {
            heap_cut(h, n, p);
            heap_cascading_cut(h, n);
        }
    }
}

int heap_decrease_key_no_replace(heap_t* h, heap_node_t* n)
{
    /* No tests that the value hasn't actually increased.  Change *
     * occurs in place, so the check is not possible here.  The   *
     * user is completely responsible for ensuring that they      *
     * don't fubar the queue.                                     */

    heap_node_t* p;

    p = n->parent;

    if (p && (h->compare(n->data, p->data) < 0)) {
        heap_cut(h, n, p);
        heap_cascading_cut(h, p);
    }
    if (h->compare(n->data, h->min->data) < 0) {
        h->min = n;
    }

    return 0;
}

void heap_node_delete(heap_t *h, heap_node_t *hn)
{
    heap_node_t *next;

    hn->prev->next = NULL;
    while (hn)
    {
        if (hn->child)
        {
            heap_node_delete(h, hn->child);
        }
        next = hn->next;
        if (h->datum_delete)
        {
            h->datum_delete(hn->data);
        }
        free(hn);
        hn = next;
    }
}

void heap_delete(heap_t* h)
{
    if (h->min)
    {
        heap_node_delete(h, h->min);
    }
    h->min = NULL;
    h->size = 0;
    h->compare = NULL;
    h->datum_delete = NULL;
}