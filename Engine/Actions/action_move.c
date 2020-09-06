#include "action_constructors.h"
#include "game_object.h"
#include "transforms.h"
#include "platform_adapter.h"
#include "string_builder.h"
#include "utils.h"

struct ActionMove {
    ACTION_OBJECT;
    Vector2D start_position;
    Vector2D end_position;
    Vector2D translation;
};

void action_move_destroy(void *obj)
{
}

char *action_move_by_describe(void *obj)
{
    struct ActionMove *action = (struct ActionMove *)obj;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "length: ");
    sb_append_float(sb, action->length, 3);
    sb_append_string(sb, " start: ");
    sb_append_vector2d(sb, action->start_position);
    sb_append_string(sb, " translation: ");
    sb_append_vector2d(sb, action->translation);

    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

char *action_move_to_describe(void *obj)
{
    struct ActionMove *action = (struct ActionMove *)obj;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "length: ");
    sb_append_float(sb, action->length, 3);
    sb_append_string(sb, " start: ");
    sb_append_vector2d(sb, action->start_position);
    sb_append_string(sb, " end: ");
    sb_append_vector2d(sb, vec_vec_add(action->start_position, action->translation));

    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

void action_move_by_start(ActionObject *action, GameObject *go)
{
    struct ActionMove *self = (struct ActionMove*)action;
    self->start_position = go->position;
}

void action_move_to_start(ActionObject *action, GameObject *go)
{
    struct ActionMove *self = (struct ActionMove*)action;
    self->start_position = go->position;
    self->translation = vec_vec_subtract(self->end_position, self->start_position);
}

Float action_move_update(ActionObject *action, GameObject *go, Float dt_s)
{
    struct ActionMove *self = (struct ActionMove*)action;
    self->position = self->position + (dt_s / self->length);
    go->position = vec_vec_add(self->start_position, vec((Number)(self->translation.x * self->position), (Number)(self->translation.y * self->position)));

    return self->position > 1.f ? (self->position - 1.f) * self->length : 0.f;
}

void action_move_finish(ActionObject *action, GameObject *go)
{
    struct ActionMove *self = (struct ActionMove*)action;
    go->position = vec_vec_add(self->start_position, vec(self->translation.x, self->translation.y));
}

static ActionObjectType ActionMoveByType = {
    { { "ActionMoveBy", &action_move_destroy, &action_move_by_describe } },
    &action_move_by_start,
    &action_move_update,
    &action_move_finish
};

static ActionObjectType ActionMoveToType = {
    { { "ActionMoveTo", &action_move_destroy, &action_move_to_describe } },
    &action_move_to_start,
    &action_move_update,
    &action_move_finish
};

ActionObject *action_move_by_create(Vector2D movement, Float length)
{
    struct ActionMove *object = platform_calloc(1, sizeof(struct ActionMove));
    object->length = length;
    object->translation = movement;
    object->w_type = &ActionMoveByType;
    
    return (ActionObject *)object;
}

ActionObject *action_move_to_create(Vector2D position, Float length)
{
    struct ActionMove *object = platform_calloc(1, sizeof(struct ActionMove));
    object->length = length;
    object->end_position = position;
    object->w_type = &ActionMoveToType;

    return (ActionObject *)object;
}

