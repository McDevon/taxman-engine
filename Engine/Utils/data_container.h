#ifndef data_container_h
#define data_container_h

#include "base_object.h"

typedef struct DataContainer {
    BASE_OBJECT;
    void *data;
} DataContainer;

/**
    DataContainer holds a pointer that is freed with platform_free() when the object is destroyed.
 */
DataContainer *data_container_create(void *data);

#endif /* data_container_h */
