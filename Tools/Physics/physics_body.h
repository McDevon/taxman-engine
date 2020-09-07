#ifndef physics_object_component_h
#define physics_object_component_h

#include <stdio.h>
#include "engine.h"

typedef struct PhysicsBody {
    GAME_OBJECT_COMPONENT;
    Vector2D velocity;
    Vector2D control_movement;
    Rect2D body_rect;
    uint32_t collision_mask;
    Bool collision_dir[4];
    Bool gravity_affects;
} PhysicsBody;

extern GameObjectComponentType PhysicsBodyComponentType;

PhysicsBody *pbd_create(void);

void pbd_set_body_rect_to_parent(PhysicsBody *self);

#endif /* physics_object_component_h */
