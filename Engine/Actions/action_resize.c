#include "action_constructors.h"
#include "game_object.h"
#include "transforms.h"
#include "platform_adapter.h"
#include "engine_log.h"
#include "string_builder.h"
#include "utils.h"

struct ActionResize {
    ACTION_OBJECT;
    Size2D start_size;
    Size2D end_size;
    Size2D change;
};

void action_resize_destroy(void *obj)
{
    
}

char *action_resize_describe(void *obj)
{
    struct ActionResize *action = (struct ActionResize *)obj;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "length: ");
    sb_append_float(sb, action->length, 3);
    sb_append_string(sb, " start: ");
    sb_append_size2d(sb, action->start_size);
    sb_append_string(sb, " target: ");
    sb_append_size2d(sb, action->end_size);

    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

void action_resize_to_start(ActionObject *action, GameObject *go)
{
    struct ActionResize *self = (struct ActionResize*)action;
    self->start_size = go->size;
}

Float action_resize_update(ActionObject *action, GameObject *go, Float dt_s)
{
    struct ActionResize *self = (struct ActionResize*)action;
    Float position = self->position + (dt_s / self->length);
    self->position = min(position, 1.f);
    go->size = (Size2D) {
        self->start_size.width * (1.f - self->position) + self->end_size.width * self->position,
        self->start_size.height * (1.f - self->position) + self->end_size.height * self->position
    };
    
    return position > 1.f ? (position - 1.f) * self->length : 0.f;
}

void action_resize_finish(ActionObject *action, GameObject *go)
{
    struct ActionResize *self = (struct ActionResize*)action;
    go->size = self->end_size;
}

static ActionObjectType ActionResizeToType = {
    { { "ActionResizeTo", &action_resize_destroy, &action_resize_describe } },
    &action_resize_to_start,
    &action_resize_update,
    &action_resize_finish
};

ActionObject *action_resize_to_create(Size2D size, Float length)
{
    struct ActionResize *object = platform_calloc(1, sizeof(struct ActionResize));
    object->length = length;
    object->end_size = size;
    object->w_type = &ActionResizeToType;

    return (ActionObject *)object;
}

