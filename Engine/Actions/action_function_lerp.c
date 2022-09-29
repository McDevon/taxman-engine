#include "action_constructors.h"
#include "game_object.h"
#include "transforms.h"
#include "platform_adapter.h"
#include "string_builder.h"
#include "utils.h"
#include "array_list.h"

struct ActionFunctionLerp {
    ACTION_OBJECT;
    void (*callback)(void *obj, void *context, Float position);
    void *context;
    Float start;
    Float end;
};

void action_function_lerp_destroy(void *obj)
{
    struct ActionFunctionLerp *self = (struct ActionFunctionLerp *)obj;
    if (self->context) {
        destroy(self->context);
        self->context = NULL;
    }
}

char *action_function_lerp_describe(void *obj)
{
    struct ActionFunctionLerp *action = (struct ActionFunctionLerp *)obj;
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

void action_function_lerp_start(ActionObject *action, GameObject *go)
{
}

float action_function_lerp_f_lerp(Float a, Float b, Float f)
{
    return a * (1.f - f) + b * f;
}

Float action_function_lerp_update(ActionObject *action, GameObject *go, Float dt_s)
{
    struct ActionFunctionLerp *self = (struct ActionFunctionLerp*)action;
    
    self->position = self->position + (dt_s / self->length);
    Float position = self->position;
    self->callback(go, self->context, self->start * (1.f - position) + self->end * position);
    
    return self->position > 1.f ? (self->position - 1.f) * self->length : 0.f;
}

void action_function_lerp_finish(ActionObject *action, GameObject *go)
{
    struct ActionFunctionLerp *self = (struct ActionFunctionLerp*)action;
    self->callback(go, self->context, self->end);
}

static ActionObjectType ActionFunctionLerpType = {
    { { "ActionFunctionLerp", &action_function_lerp_destroy, &action_function_lerp_describe } },
    &action_function_lerp_start,
    &action_function_lerp_update,
    &action_function_lerp_finish
};

ActionObject *action_function_lerp_create(void (*callback)(void *obj, void *context, Float position), void *context, Float length, Float start, Float end)
{
    struct ActionFunctionLerp *object = platform_calloc(1, sizeof(struct ActionFunctionLerp));
    object->callback = callback;
    object->context = context;
    object->length = length;
    object->start = start;
    object->end = end;
    object->w_type = &ActionFunctionLerpType;

    return (ActionObject *)object;
}

