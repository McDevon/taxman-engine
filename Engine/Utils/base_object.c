#include "base_object.h"
#include "string_builder.h"
#include "engine_log.h"
#include "platform_adapter.h"
#include <stdlib.h>

void destroy(void *object)
{
    Object *target = (Object *)object;
    BaseType *type = (BaseType *)target->w_type;
    type->destroy(object);
    platform_free(object);
}

char *object_type_string(void *object)
{
    Object *target = (Object *)object;
    BaseType *type = (BaseType *)target->w_type;
    return type->type_name;
}

char *describe(void *object)
{
    Object *target = (Object *)object;
    BaseType *type = (BaseType *)target->w_type;
    return type->description(object);
}

char *describe_debug(void *object)
{
    Object *target = (Object *)object;
    BaseType *type = (BaseType *)target->w_type;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "OBJECT TYPE: ");
    sb_append_string(sb, type->type_name);
    sb_append_string(sb, " DESCRIPTION: ");
    char *description = type->description(object);
    sb_append_string(sb, description);
    
    char *debug_string = sb_get_string(sb);
    destroy(sb);
    platform_free(description);
    
    return debug_string;
}

void describe_debug_to_log(void *object)
{
    char *debug_string = describe_debug(object);
    LOG("%s", debug_string);
    platform_free(debug_string);
}
