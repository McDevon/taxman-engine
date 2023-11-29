#include "action_constructors.h"
#include "action_animator_private.h"
#include "game_object.h"
#include "transforms.h"
#include "platform_adapter.h"
#include "string_builder.h"
#include "utils.h"
#include "bezier.h"
#include <math.h>
#include <string.h>

struct ActionEase {
    ACTION_OBJECT;
    ActionObject *action_object;
    Float (*function)(Float, void *);
    void *context;
};

void action_ease_destroy(void *obj)
{
    struct ActionEase *self = (struct ActionEase *)obj;
    destroy(self->action_object);
    self->action_object = NULL;
    
    if (self->context) {
        destroy(self->context);
        self->context = NULL;
    }
}

char *action_ease_describe(void *obj)
{
    struct ActionEase *action = (struct ActionEase *)obj;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "position: ");
    sb_append_float(sb, action->position, 3);
    sb_append_string(sb, " ease pos: ");
    sb_append_float(sb, action->action_object->position, 3);
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

void action_ease_start(ActionObject *action, GameObject *go)
{
    struct ActionEase *self = (struct ActionEase*)action;
    action_call_start(self->action_object, go);
}

Float action_ease_update(ActionObject *action, GameObject *go, Float dt_s)
{
    struct ActionEase *self = (struct ActionEase*)action;
    
    ActionObject *target = self->action_object;
    
    self->position += dt_s / self->length;
    Float target_dt_s;
    
    target_dt_s = (self->function(self->position, self->context) - target->position) * target->length;
    action_call_update(target, go, target_dt_s);
    
    return max((self->position - 1.f) * self->length, 0.f);
}

void action_ease_finish(ActionObject *action, GameObject *go)
{
    struct ActionEase *self = (struct ActionEase*)action;
    action_call_finish(self->action_object, go);
}

static ActionObjectType ActionEaseType = {
    { { "ActionEase", &action_ease_destroy, &action_ease_describe } },
    &action_ease_start,
    &action_ease_update,
    &action_ease_finish
};

Float action_ease_in_fn(Float value, void *c) {
    return -1.f * cosf(value * (Float)M_PI_2) + 1.f;
}

Float action_ease_out_fn(Float value, void *c) {
    return sinf(value * (Float)M_PI_2);
}

Float action_ease_in_out_fn(Float value, void *c) {
    return -0.5f * (cosf((Float)M_PI * value) - 1.f);
}

Float action_ease_linear_fn(Float value, void *c) {
    return value;
}

Float action_ease_bezier_fn(Float value , void *c)
{
    BezierModel *model = (BezierModel *)c;
    return bezier_compute(model, value);
}

Float action_ease_bezier_precomputed_fn(Float value , void *c)
{
    BezierPrecomputed *model = (BezierPrecomputed *)c;
    return bezier_precomputed_get(model, value);
}

struct ActionEase *action_ease_create(ActionObject *action)
{
    struct ActionEase *object = platform_calloc(1, sizeof(struct ActionEase));
    object->length = action->length;
    object->action_object = action;
    object->w_type = &ActionEaseType;
    object->context = NULL;

    return object;
}

ActionObject *action_ease_in_create(ActionObject *action)
{
    struct ActionEase *object = action_ease_create(action);
    object->function = &action_ease_in_fn;
    
    return (ActionObject *)object;
}

ActionObject *action_ease_out_create(ActionObject *action)
{
    struct ActionEase *object = action_ease_create(action);
    object->function = &action_ease_out_fn;
    
    return (ActionObject *)object;
}

ActionObject *action_ease_in_out_create(ActionObject *action)
{
    struct ActionEase *object = action_ease_create(action);
    object->function = &action_ease_in_out_fn;
    
    return (ActionObject *)object;
}

ActionObject *action_ease_bezier_create(ActionObject *action, Float control_points[4])
{
    struct ActionEase *object = action_ease_create(action);
    
    if (control_points[0] == control_points[1] && control_points[2] == control_points[3]) {
        object->function = &action_ease_linear_fn;
    } else {
        object->function = &action_ease_bezier_fn;
        object->context = bezier_model_create(control_points);
    }
    
    return (ActionObject *)object;
}

ActionObject *action_ease_bezier_prec_create(ActionObject *action, Float control_points[4], size_t table_size)
{
    struct ActionEase *object = action_ease_create(action);
    
    if (control_points[0] == control_points[1] && control_points[2] == control_points[3]) {
        object->function = &action_ease_linear_fn;
    } else {
        object->function = &action_ease_bezier_precomputed_fn;
        object->context = bezier_precomputed_create(control_points, table_size);
    }
    
    return (ActionObject *)object;
}

ActionObject *action_ease_bezier_prec_table_create(ActionObject *action, Float *table, size_t table_size)
{
    struct ActionEase *object = action_ease_create(action);
    
    object->function = &action_ease_bezier_precomputed_fn;
    object->context = bezier_precomputed_create_from_table(table, table_size);
    
    return (ActionObject *)object;
}

