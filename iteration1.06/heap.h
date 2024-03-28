#ifndef HEAP_H
#define HEAP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
    struct heap_node;
    typedef struct heap_node heap_node_t;

    typedef struct heap
    {
        heap_node_t* min;
        uint32_t size;
        int32_t(*compare)(const void* key, const void* with);
        void (*datum_delete)(void*);
    } heap_t;
    void heap_init(heap_t* h,
        int32_t(*compare)(const void* key, const void* with),
        void (*datum_delete)(void*));
    void* heap_remove_min(heap_t* h);
    heap_node_t* heap_insert(heap_t* h, void* v);
    int heap_decrease_key_no_replace(heap_t* h, heap_node_t* n); // you have the heap node to avoid the linear search in the heap
    // you will call this when you want find a shortest path and u want to adjust the path
    void* heap_peek_min(heap_t* h);

    void heap_delete(heap_t* h);

#ifdef __cplusplus
}
#endif

#endif
