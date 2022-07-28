#ifndef collision_body_h
#define collision_body_h

#include "engine.h"
#include "collision_world.h"

typedef struct CollisionBody {
    GAME_OBJECT_COMPONENT;
    Rect2D body_rect;
    Vector2D control_movement;
    Vector2D velocity;
    uint8_t collision_layer;
} CollisionBody;

extern GameObjectComponentType CollisionBodyComponentType;

CollisionBody *coll_create(void);

#endif /* collision_body_h */
