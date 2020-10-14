#include "physics_body.h"

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

void pbd_start(GameObjectComponent *comp)
{
    PhysicsBody *self = (PhysicsBody *)comp;
    
    if (self->body_rect.size.width == nb_zero || self->body_rect.size.height == nb_zero) {
        pbd_set_body_rect_to_parent(self);
    }
}

GameObjectComponentType PhysicsBodyComponentType = {
    { { "PhysicsBody", &pbd_destroy, &pbd_describe } },
    NULL,
    &pbd_start,
    NULL,
    NULL
};

PhysicsBody *pbd_create()
{
    PhysicsBody *pho = (PhysicsBody *)comp_alloc(sizeof(PhysicsBody));
    
    pho->w_type = &PhysicsBodyComponentType;
    pho->gravity_affects = True;
    pho->collision_layer = 0;
    pho->collision_dir[dir_left] = False;
    pho->collision_dir[dir_right] = False;
    pho->collision_dir[dir_up] = False;
    pho->collision_dir[dir_down] = False;

    return pho;
}
