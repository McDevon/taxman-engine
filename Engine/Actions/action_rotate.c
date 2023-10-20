#include "action_constructors.h"
#include "game_object.h"
#include "transforms.h"
#include "platform_adapter.h"
#include "string_builder.h"
#include "utils.h"

struct ActionRotate {
    ACTION_OBJECT;
    Float start_rotation;
    Float end_rotation;
    Float offset;
};

void action_rotate_destroy(void *obj)
{
}

char *action_rotate_by_describe(void *obj)
{
    struct ActionRotate *action = (struct ActionRotate *)obj;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "length: ");
    sb_append_float(sb, action->length, 3);
    sb_append_string(sb, " start: ");
    sb_append_number(sb, action->start_rotation);
    sb_append_string(sb, " offset: ");
    sb_append_number(sb, action->offset);

    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

char *action_rotate_to_describe(void *obj)
{
    struct ActionRotate *action = (struct ActionRotate *)obj;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "length: ");
    sb_append_float(sb, action->length, 3);
    sb_append_string(sb, " start: ");
    sb_append_number(sb, action->start_rotation);
    sb_append_string(sb, " end: ");
    sb_append_number(sb, action->start_rotation + action->offset);

    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

void action_rotate_by_start(ActionObject *action, GameObject *go)
{
    struct ActionRotate *self = (struct ActionRotate*)action;
    self->start_rotation = go->rotation;
}

void action_rotate_to_start(ActionObject *action, GameObject *go)
{
    struct ActionRotate *self = (struct ActionRotate*)action;
    self->start_rotation = go->rotation;
    self->offset = self->end_rotation - self->start_rotation;
}

Float action_rotate_update(ActionObject *action, GameObject *go, Float dt_s)
{
    struct ActionRotate *self = (struct ActionRotate*)action;
    Float position = self->position + (dt_s / self->length);
    self->position = min(position, 1.f);
    go->rotation = self->start_rotation + self->offset * position;
    
    return position > 1.f ? (position - 1.f) * self->length : 0.f;
}

void action_rotate_finish(ActionObject *action, GameObject *go)
{
    struct ActionRotate *self = (struct ActionRotate*)action;
    go->rotation = self->start_rotation + self->offset;
}

static ActionObjectType ActionRotateByType = {
    { { "ActionRotateBy", &action_rotate_destroy, &action_rotate_by_describe } },
    &action_rotate_by_start,
    &action_rotate_update,
    &action_rotate_finish
};

static ActionObjectType ActionRotateToType = {
    { { "ActionRotateTo", &action_rotate_destroy, &action_rotate_to_describe } },
    &action_rotate_to_start,
    &action_rotate_update,
    &action_rotate_finish
};

ActionObject *action_rotate_by_create(Float offset, Float length)
{
    struct ActionRotate *object = platform_calloc(1, sizeof(struct ActionRotate));
    object->length = length;
    object->offset = offset;
    object->w_type = &ActionRotateByType;

    return (ActionObject *)object;
}

ActionObject *action_rotate_to_create(Float target, Float length)
{
    struct ActionRotate *object = platform_calloc(1, sizeof(struct ActionRotate));
    object->length = length;
    object->end_rotation = target;
    object->w_type = &ActionRotateToType;

    return (ActionObject *)object;
}

