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
    if (self->size.width == nb_zero || self->size.height == nb_zero) {
        self->object_offset = vec(nb_mul(parent->anchor.x, parent->size.width),
                                  nb_mul(parent->anchor.y, parent->size.height));
        self->size = (Size2D) { parent->size.width, parent->size.height };
    }
}

void pbd_set_position_to_parent(PhysicsBody *self)
{
    GameObject *parent = comp_get_parent(self);
    self->position = vec_round(vec_vec_subtract(parent->position, self->object_offset));
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
    
    if (self->size.width == nb_zero || self->size.height == nb_zero) {
        pbd_set_body_rect_to_parent(self);
    }
    
    pbd_set_position_to_parent(self);
    pbd_add_to_world(self);
}

void pbd_update(GameObjectComponent *comp, Number dt_ms)
{
    PhysicsBody *self = (PhysicsBody *)comp;
    GameObject *parent = comp_get_parent(self);
    
    parent->position = vec_vec_add(self->position, self->object_offset);
}

GameObjectComponentType PhysicsBodyComponentType = {
    { { "PhysicsBody", &pbd_destroy, &pbd_describe } },
    NULL,
    &pbd_obj_will_be_removed,
    &pbd_start,
    &pbd_update,
    NULL
};

PhysicsBody *pbd_create()
{
    PhysicsBody *pho = (PhysicsBody *)comp_alloc(sizeof(PhysicsBody));
    
    pho->w_type = &PhysicsBodyComponentType;
    pho->crush_override_callback = NULL;
    pho->w_callback_context = NULL;
    pho->position = vec_zero();
    pho->size = (Size2D){ nb_zero, nb_zero };
    pho->object_offset = vec_zero();
    pho->remainder_movement = vec_zero();
    pho->collision_layer = 0;
    pho->collision_directions = directions_all;
    pho->dynamic = false;
    pho->trigger = false;

    return pho;
}

void pbd_move_dynamic(PhysicsBody *physics_body, Vector2D movement, pbd_collision_callback_t callback, void *collision_context)
{
    if (physics_body->w_world) {
        world_pbd_move_dynamic(physics_body->w_world, physics_body, movement, callback, collision_context);
    }
}

void pbd_move_static(PhysicsBody *physics_body, Vector2D movement)
{
    if (physics_body->w_world) {
        world_pbd_move_static(physics_body->w_world, physics_body, movement);
    }
}

void pbd_crush(PhysicsBody *physics_body, PhysicsBody *crushing_body, Direction direction, void *collision_context)
{
    if (physics_body->crush_override_callback) {
        physics_body->crush_override_callback(physics_body, crushing_body, direction, physics_body->w_callback_context);
    } else {
        go_schedule_destroy(comp_get_parent(physics_body));
    }
}

void pbd_pushed(PhysicsBody *physics_body, PhysicsBody *pushing_body, Direction moving_direction)
{
    if (physics_body->push_callback) {
        physics_body->push_callback(physics_body, pushing_body, moving_direction, physics_body->w_callback_context);
    }
}

inline Number pbd_left(PhysicsBody *physics_body)
{
    return physics_body->position.x;
}

inline Number pbd_right(PhysicsBody *physics_body)
{
    return physics_body->position.x + physics_body->size.width - nb_one;
}

inline Number pbd_top(PhysicsBody *physics_body)
{
    return physics_body->position.y;
}

inline Number pbd_bottom(PhysicsBody *physics_body)
{
    return physics_body->position.y + physics_body->size.height - nb_one;
}

inline bool pbd_overlap(PhysicsBody *pbd_a, PhysicsBody *pbd_b)
{
    return !(pbd_right(pbd_a) < pbd_left(pbd_b)
             || pbd_bottom(pbd_a) < pbd_top(pbd_b)
             || pbd_left(pbd_a) > pbd_right(pbd_b)
             || pbd_top(pbd_a) > pbd_bottom(pbd_b));
}

bool pbd_overlap_in_position(PhysicsBody *pbd_a, PhysicsBody *pbd_b, Vector2D pbd_a_position)
{
    return !(pbd_a_position.x + pbd_a->size.width - nb_one < pbd_left(pbd_b)
             || pbd_a_position.y + pbd_a->size.height - nb_one < pbd_top(pbd_b)
             || pbd_a_position.x > pbd_right(pbd_b)
             || pbd_a_position.y > pbd_bottom(pbd_b));
}

inline bool pbd_vertical_overlap(PhysicsBody *pbd_a, PhysicsBody *pbd_b)
{
    return !(pbd_bottom(pbd_a) < pbd_top(pbd_b)
             || pbd_top(pbd_a) > pbd_bottom(pbd_b));
}
