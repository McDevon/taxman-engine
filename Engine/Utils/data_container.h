#ifndef data_container_h
#define data_container_h

#include "base_object.h"

typedef struct DataContainer {
    BASE_OBJECT;
    void *data;
} DataContainer;

DataContainer *data_container_create(void *data);

#endif /* data_container_h */
