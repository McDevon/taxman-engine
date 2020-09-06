#include "action_constructors.h"
#include "game_object.h"
#include "transforms.h"
#include "platform_adapter.h"
#include "string_builder.h"
#include "utils.h"
#include "array_list.h"

struct ActionFunction {
    ACTION_OBJECT;
    void (*callback)(void *obj, void *context, Float position);
    void *context;
};

void action_function_destroy(void *obj)
{
    struct ActionFunction *self = (struct ActionFunction *)obj;
    if (self->context) {
        destroy(self->context);
        self->context = NULL;
    }
}

char *action_function_describe(void *obj)
{
    struct ActionFunction *action = (struct ActionFunction *)obj;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "context: ");
    if (action->context) {
        char *context_desc = describe(action->context);
        sb_append_string(sb, context_desc);
        platform_free(context_desc);
    } else {
        sb_append_string(sb, "NULL");
    }
    
    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

void action_function_start(ActionObject *action, GameObject *go)
{
}

Float action_function_update(ActionObject *action, GameObject *go, Float dt_s)
{
    struct ActionFunction *self = (struct ActionFunction*)action;
    
    self->position = self->position + (dt_s / self->length);
    self->callback(go, self->context, self->position);
    
    return self->position > 1.f ? (self->position - 1.f) * self->length : 0.f;
}

void action_function_finish(ActionObject *action, GameObject *go)
{
    struct ActionFunction *self = (struct ActionFunction*)action;
    self->callback(go, self->context, 1.f);
}

static ActionObjectType ActionFunctionType = {
    { { "ActionFunction", &action_function_destroy, &action_function_describe } },
    &action_function_start,
    &action_function_update,
    &action_function_finish
};

ActionObject *action_function_create(void (*callback)(void *obj, void *context, Float position), void *context, Float length)
{
    struct ActionFunction *object = platform_calloc(1, sizeof(struct ActionFunction));
    object->callback = callback;
    object->context = context;
    object->length = length;
    object->w_type = &ActionFunctionType;

    return (ActionObject *)object;
}

