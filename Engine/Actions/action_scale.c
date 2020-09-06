#include "action_constructors.h"
#include "game_object.h"
#include "transforms.h"
#include "platform_adapter.h"
#include "engine_log.h"
#include "string_builder.h"
#include "utils.h"

struct ActionScale {
    ACTION_OBJECT;
    Vector2D start_scale;
    Vector2D end_scale;
    Vector2D change;
};

void action_scale_destroy(void *obj)
{
}

char *action_scale_describe(void *obj)
{
    struct ActionScale *action = (struct ActionScale *)obj;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "length: ");
    sb_append_float(sb, action->length, 3);
    sb_append_string(sb, " start: ");
    sb_append_vector2d(sb, action->start_scale);
    sb_append_string(sb, " target: ");
    sb_append_vector2d(sb, action->end_scale);

    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

void action_scale_by_start(ActionObject *action, GameObject *go)
{
    struct ActionScale *self = (struct ActionScale*)action;
    self->start_scale = go->scale;
    self->end_scale = vec(nb_mul(go->scale.x, self->change.x), nb_mul(go->scale.y, self->change.y));
}

void action_scale_to_start(ActionObject *action, GameObject *go)
{
    struct ActionScale *self = (struct ActionScale*)action;
    self->start_scale = go->scale;
}

Float action_scale_update(ActionObject *action, GameObject *go, Float dt_s)
{
    struct ActionScale *self = (struct ActionScale*)action;
    Float position = self->position + (dt_s / self->length);
    self->position = min(position, 1.f);
    go->scale = vec_f_lerp(self->start_scale, self->end_scale, position);
    
    return position > 1.f ? (position - 1.f) * self->length : 0.f;
}

void action_scale_finish(ActionObject *action, GameObject *go)
{
    struct ActionScale *self = (struct ActionScale*)action;
    go->scale = self->end_scale;
}

static ActionObjectType ActionScaleByType = {
    { { "ActionScaleBy", &action_scale_destroy, &action_scale_describe } },
    &action_scale_by_start,
    &action_scale_update,
    &action_scale_finish
};

static ActionObjectType ActionScaleToType = {
    { { "ActionScaleTo", &action_scale_destroy, &action_scale_describe } },
    &action_scale_to_start,
    &action_scale_update,
    &action_scale_finish
};

ActionObject *action_scale_by_create(Vector2D scale, Float length)
{
    struct ActionScale *object = platform_calloc(1, sizeof(struct ActionScale));
    object->length = length;
    object->change = scale;
    object->w_type = &ActionScaleByType;

    return (ActionObject *)object;
}

ActionObject *action_scale_to_create(Vector2D scale, Float length)
{
    struct ActionScale *object = platform_calloc(1, sizeof(struct ActionScale));
    object->length = length;
    object->end_scale = scale;
    object->w_type = &ActionScaleToType;

    return (ActionObject *)object;
}

