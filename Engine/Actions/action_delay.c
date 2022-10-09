#include "action_constructors.h"
#include "game_object.h"
#include "transforms.h"
#include "platform_adapter.h"
#include "string_builder.h"
#include "utils.h"
#include "array_list.h"

struct ActionDelay {
    ACTION_OBJECT;
    Float timer;
};

void action_delay_destroy(void *obj)
{
}

char *action_delay_describe(void *obj)
{
    struct ActionDelay *action = (struct ActionDelay *)obj;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "length: ");
    sb_append_float(sb, action->length, 3);
    sb_append_string(sb, " timer: ");
    sb_append_float(sb, action->timer, 3);
    
    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

void action_delay_start(ActionObject *action, GameObject *go)
{
    struct ActionDelay *self = (struct ActionDelay*)action;
    self->timer = 0.f;    
}

Float action_delay_update(ActionObject *action, GameObject *go, Float dt_s)
{
    struct ActionDelay *self = (struct ActionDelay*)action;
    
    self->timer += dt_s;
    self->position = self->timer / self->length;
    
    return max(self->timer - self->length, 0.f);
}

static ActionObjectType ActionDelayType = {
    { { "ActionDelay", &action_delay_destroy, &action_delay_describe } },
    &action_delay_start,
    &action_delay_update,
    NULL
};

ActionObject *action_delay_create(Float length)
{
    struct ActionDelay *object = platform_calloc(1, sizeof(struct ActionDelay));
    object->length = length;
    object->w_type = &ActionDelayType;

    return (ActionObject *)object;
}

