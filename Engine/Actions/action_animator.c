#include "action_animator.h"
#include "platform_adapter.h"
#include "game_object_component.h"

typedef struct Act {
    GAME_OBJECT_COMPONENT;
    ActionObject *action_object;
} Act;

void act_destroy(void *comp)
{
    Act *action = (Act *)comp;
    comp_destroy(comp);
    
    destroy(action->action_object);
}

char *act_describe(void *comp)
{
    return comp_describe(comp);
}

void act_added(GameObjectComponent *comp)
{

}

void act_start(GameObjectComponent *comp)
{
    Act *self = (Act *)comp;
    
    if (!self->action_object) {
        return;
    }
    
    ActionObjectType *a_type = (ActionObjectType *)self->action_object->w_type;
    if (a_type->start) {
        a_type->start(self->action_object, comp_get_parent(self));        
    }
}

void act_update(GameObjectComponent *comp, Number dt_ms)
{
    Act *self = (Act *)comp;
    ActionObject *action = self->action_object;
    
    if (!action || action->position >= 1.f) {
        comp_schedule_destroy(self);
        return;
    }
    
    Float dt_s = nb_to_float(dt_ms) / 1000.f;
    
    GameObject *object = comp_get_parent(self);
    
    ActionObjectType *a_type = (ActionObjectType *)action->w_type;
    a_type->update(action, object, dt_s);
    
    if (action->position >= 1.f) {
        action->position = 1.f;
        if (a_type->finish) {
            a_type->finish(action, object);
        }
    }
}

GameObjectComponentType ActType = {
    { { "Act [Component]", &act_destroy, &act_describe } },
    &act_added,
    NULL,
    &act_start,
    &act_update,
    NULL
};

Act *act_create(ActionObject *action_object)
{
    Act *act = (Act *)comp_alloc(sizeof(Act));
    act->w_type = &ActType;
    act->action_object = action_object;
    
    return act;
}

void action_call_start(ActionObject *action, GameObject *go)
{
    action->position = 0.f;
    ActionObjectType *a_type = (ActionObjectType *)action->w_type;
    if (a_type->start) {
        a_type->start(action, go);
    }
}

Float action_call_update(ActionObject *action, GameObject *go, Float dt_s)
{
    ActionObjectType *a_type = (ActionObjectType *)action->w_type;
    return a_type->update(action, go, dt_s);
}

void action_call_finish(ActionObject *action, GameObject *go)
{
    action->position = 1.f;
    ActionObjectType *a_type = (ActionObjectType *)action->w_type;
    if (a_type->finish) {
        a_type->finish(action, go);
    }
}
