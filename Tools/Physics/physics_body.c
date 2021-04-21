#include "physics_body.h"
#include "physics_world.h"

void pbd_destroy(void *comp)
{    
    comp_destroy(comp);
}

char *pbd_describe(void *comp)
{
    return comp_describe(comp);
}

void pbd_set_body_rect_to_parent(PhysicsBody *self)
{
    GameObject *parent = comp_get_parent(self);
    if (self->body_rect.size.width == nb_zero || self->body_rect.size.height == nb_zero) {
        self->body_rect = rect_make(-nb_mul(parent->anchor.x, parent->size.width),
                                    -nb_mul(parent->anchor.y, parent->size.height),
                                    parent->size.width,
                                    parent->size.height);
    }
}

void pbd_add_to_world(PhysicsBody* self)
{
    GameObject *obj = comp_get_parent(self);
    GameObject *parent = go_get_parent(obj);
    if (!parent) {
        return;
    }
    GameObjectComponent *world_comp = go_get_component(parent, &PhysicsWorldComponentType);
    if (!world_comp) {
        return;
    }
    world_add_child((PhysicsWorld *)world_comp, obj);
}

void pbd_obj_will_be_removed(GameObjectComponent *comp)
{
    GameObject *obj = comp_get_parent(comp);
    GameObject *parent = go_get_parent(obj);
    if (!parent) {
        return;
    }
    GameObjectComponent *world_comp = go_get_component(parent, &PhysicsWorldComponentType);
    if (!world_comp) {
        return;
    }
    world_remove_object_from_world(obj);
}

void pbd_start(GameObjectComponent *comp)
{
    PhysicsBody *self = (PhysicsBody *)comp;
    
    if (self->body_rect.size.width == nb_zero || self->body_rect.size.height == nb_zero) {
        pbd_set_body_rect_to_parent(self);
    }
    
    pbd_add_to_world(self);
}

GameObjectComponentType PhysicsBodyComponentType = {
    { { "PhysicsBody", &pbd_destroy, &pbd_describe } },
    NULL,
    &pbd_obj_will_be_removed,
    &pbd_start,
    NULL,
    NULL
};

PhysicsBody *pbd_create()
{
    PhysicsBody *pho = (PhysicsBody *)comp_alloc(sizeof(PhysicsBody));
    
    pho->w_type = &PhysicsBodyComponentType;
    pho->gravity_affects = true;
    pho->collision_layer = 0;
    pho->collision_dir[dir_left] = false;
    pho->collision_dir[dir_right] = false;
    pho->collision_dir[dir_up] = false;
    pho->collision_dir[dir_down] = false;

    return pho;
}
