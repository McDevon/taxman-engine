#include "data_container.h"
#include "platform_adapter.h"

void data_container_destroy(void *value)
{
    DataContainer *dc = (DataContainer *)value;
    if (dc->data) {
        platform_free(dc->data);
    }
}

char *data_container_describe(void *value)
{
    return platform_strdup("{}");
}

static BaseType DataContainerType = { "DataContainer", &data_container_destroy, &data_container_describe };

DataContainer *data_container_create(void *data)
{
    DataContainer *dc = platform_calloc(1, sizeof(DataContainer));
    dc->w_type = &DataContainerType;
    dc->data = data;
    
    return dc;
}
