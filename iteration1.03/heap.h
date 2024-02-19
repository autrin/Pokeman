#ifndef HEAP.H
#define HEAP .H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

    typedef struct heap
    {
        heap_node_t *min;
        uint32_t size;
        int32_t (*compare)(const void *key, const void *with);
        void (*datum_delete)(void *);
    } heap_t;

#ifdef __cplusplus
}
#endif

#endif
