#include "collision_body.h"
#include "collision_world.h"

void coll_destroy(void *comp)
{
    comp_destroy(comp);
}

char *coll_describe(void *comp)
{
    return comp_describe(comp);
}

void coll_set_body_rect_to_parent(CollisionBody *self)
{
    GameObject *parent = comp_get_parent(self);
    if (self->body_rect.size.width == 0.f || self->body_rect.size.height == 0.f) {
        self->body_rect = rect_make(-(parent->anchor.x * parent->size.width), -(parent->anchor.y * parent->size.height), parent->size.width, parent->size.height);
    }
}

void coll_add_to_world(CollisionBody* self)
{
    GameObject *obj = comp_get_parent(self);
    GameObject *parent = go_get_parent(obj);
    if (!parent) {
        return;
    }
    GameObjectComponent *world_comp = go_get_component(parent, &CollisionWorldComponentType);
    if (!world_comp) {
        return;
    }
    c_world_add_child((CollisionWorld *)world_comp, obj);
}

void coll_obj_will_be_removed(GameObjectComponent *comp)
{
    GameObject *obj = comp_get_parent(comp);
    GameObject *parent = go_get_parent(obj);
    if (!parent) {
        return;
    }
    GameObjectComponent *world_comp = go_get_component(parent, &CollisionWorldComponentType);
    if (!world_comp) {
        return;
    }
    c_world_remove_object_from_world(obj);
}

void coll_start(GameObjectComponent *comp)
{
    CollisionBody *self = (CollisionBody *)comp;
    
    if (self->body_rect.size.width == 0.f || self->body_rect.size.height == 0.f) {
        coll_set_body_rect_to_parent(self);
    }
    
    coll_add_to_world(self);
}

GameObjectComponentType CollisionBodyComponentType = {
    { { "CollisionBody", &coll_destroy, &coll_describe } },
    NULL,
    &coll_obj_will_be_removed,
    &coll_start,
    NULL,
    NULL
};

CollisionBody *coll_create()
{
    CollisionBody *coll = (CollisionBody *)comp_alloc(sizeof(CollisionBody));
    
    coll->w_type = &CollisionBodyComponentType;
    coll->body_rect = rect_make(0.f, 0.f, 0.f, 0.f);
    coll->velocity = vec_zero();
    coll->control_movement = vec_zero();
    coll->collision_layer = 0;

    return coll;
}
