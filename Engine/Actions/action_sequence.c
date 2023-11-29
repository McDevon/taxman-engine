#include "action_constructors.h"
#include "action_animator_private.h"
#include "game_object.h"
#include "transforms.h"
#include "platform_adapter.h"
#include "string_builder.h"
#include "utils.h"
#include "array_list.h"

struct ActionSequence {
    ACTION_OBJECT;
    ArrayList *actions;
    size_t index;
};

void action_sequence_destroy(void *obj)
{
    struct ActionSequence *self = (struct ActionSequence *)obj;
    destroy(self->actions);
    self->actions = NULL;
}

char *action_sequence_describe(void *obj)
{
    struct ActionSequence *action = (struct ActionSequence *)obj;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "count: ");
    sb_append_int(sb, (int)list_count(action->actions));
    sb_append_string(sb, " index: ");
    sb_append_int(sb, (int)action->index);
    
    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

void action_sequence_start(ActionObject *action, GameObject *go)
{
    struct ActionSequence *self = (struct ActionSequence*)action;
    self->index = 0;
    action_call_start(list_get(self->actions, self->index), go);
}

Float action_sequence_update(ActionObject *action, GameObject *go, Float dt_s)
{
    struct ActionSequence *self = (struct ActionSequence*)action;
    
    ActionObject *current_action = list_get(self->actions, self->index);
    Float available_time = dt_s;
    
    size_t actions_count = list_count(self->actions);
    
    while (available_time > 0.f && (self->index < actions_count)) {
        available_time = action_call_update(current_action, go, available_time);
        if (current_action->position >= 1.f) {
            action_call_finish(current_action, go);
            if (++self->index < actions_count) {
                current_action = list_get(self->actions, self->index);
                action_call_start(current_action, go);
            }
        }
    }
    
    Float position = 0.f;
    for (size_t i = 0; i < self->index; ++i) {
        ActionObject *action = list_get(self->actions, i);
        position += action->length;
    }
    if (self->index < actions_count) {
        position += current_action->position * current_action->length;
    }
    self->position = self->length > 0.f ? position / self->length : 0.f;
    
    return available_time;
}

static ActionObjectType ActionSequenceType = {
    { { "ActionSequence", &action_sequence_destroy, &action_sequence_describe } },
    &action_sequence_start,
    &action_sequence_update,
    NULL
};

ActionObject *action_sequence_create(ArrayList *actions)
{
    struct ActionSequence *object = platform_calloc(1, sizeof(struct ActionSequence));
    Float length = 0.f;
    for (size_t i = 0; i < list_count(actions); ++i) {
        ActionObject *action = list_get(actions, i);
        length += action->length;
    }
    object->length = length;
    object->actions = actions;
    object->w_type = &ActionSequenceType;

    return (ActionObject *)object;
}

