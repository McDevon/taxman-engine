#include "action_constructors.h"
#include "action_animator_private.h"
#include "game_object.h"
#include "transforms.h"
#include "platform_adapter.h"
#include "string_builder.h"
#include "utils.h"

struct ActionRepeat {
    ACTION_OBJECT;
    ActionObject *action_object;
    int count;
    int counter;
};

void action_repeat_destroy(void *obj)
{
    struct ActionRepeat *self = (struct ActionRepeat *)obj;
    destroy(self->action_object);
    self->action_object = NULL;
}

char *action_repeat_describe(void *obj)
{
    struct ActionRepeat *action = (struct ActionRepeat *)obj;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "count: ");
    sb_append_int(sb, action->count);
    sb_append_string(sb, " counter: ");
    sb_append_int(sb, action->counter);
    sb_append_string(sb, " action [");
    sb_append_string(sb, object_type_string(action->action_object));
    sb_append_string(sb, " ]: ");
    char *action_desc = describe(action->action_object);
    sb_append_string(sb, action_desc);
    platform_free(action_desc);
    
    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

void action_repeat_start(ActionObject *action, GameObject *go)
{
    struct ActionRepeat *self = (struct ActionRepeat*)action;
    self->counter = 0;
    
    action_call_start(self->action_object, go);
}

Float action_repeat_update(ActionObject *action, GameObject *go, Float dt_s)
{
    struct ActionRepeat *self = (struct ActionRepeat*)action;
    
    Float available_time = dt_s;
    
    while (available_time > 0.f && (self->counter < self->count || self->count == 0)) {
        available_time = action_call_update(self->action_object, go, available_time);
        if (self->action_object->position >= 1.f) {
            action_call_finish(self->action_object, go);
            if (self->count == 0 || self->counter++ < self->count) {
                action_call_start(self->action_object, go);
            }
        }
    }
    
    self->position = self->length <= 0.f ? 0.f : ((self->counter + self->action_object->position) * self->action_object->length) / self->length;
    
    return available_time;
}

static ActionObjectType ActionRepeatType = {
    { { "ActionRepeat", &action_repeat_destroy, &action_repeat_describe } },
    &action_repeat_start,
    &action_repeat_update,
    NULL
};

ActionObject *action_repeat_create(ActionObject *action, int count)
{
    struct ActionRepeat *object = platform_calloc(1, sizeof(struct ActionRepeat));
    object->length = count == 0 ? __FLT_MAX__ : action->length * count;
    object->count = count;
    object->action_object = action;
    object->w_type = &ActionRepeatType;

    return (ActionObject *)object;
}

