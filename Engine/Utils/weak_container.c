#include "weak_container.h"
#include "platform_adapter.h"

void weak_container_destroy(void *object)
{
}

char *weak_container_describe(void *object)
{
    return platform_strdup("[]");
}

BaseType WeakContainerType = { "WeakContainer", &weak_container_destroy, &weak_container_describe };

WeakContainer *weak_container_create(void *item)
{
    WeakContainer *cont = platform_calloc(1, sizeof(WeakContainer));
    cont->w_type = &WeakContainerType;
    cont->w_item = item;
    return cont;
}
