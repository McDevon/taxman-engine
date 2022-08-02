#ifndef weak_container_h
#define weak_container_h

#include "base_object.h"

typedef struct WeakContainer {
    BASE_OBJECT;
    void *w_item;
} WeakContainer;

WeakContainer *weak_container_create(void *item);

#endif /* weak_container_h */
