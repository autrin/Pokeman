#ifndef HEAP.H
#define HEAP .H

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

#ifdef __cplusplus
}
#endif

#endif
