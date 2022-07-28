#ifndef collision_world_h
#define collision_world_h

#include "engine.h"
#include "tilemap.h"

struct CollisionBody;
extern GameObjectComponentType CollisionWorldComponentType;

typedef void (collision_world_callback_t)(struct CollisionBody *obj_a, struct CollisionBody *obj_b, void *context);

/**
 Physics World is a tool for creating easy overlap collision detection for AABB colliders.
 
 Features:
 - AABB colliders
 - Collision matrix
 - One-directional sweep-and-prune algorithm to ease the detection load
 */
typedef struct CollisionWorld CollisionWorld;

CollisionWorld *c_world_create(void *callback_context, collision_world_callback_t *collision_callback, uint16_t collision_masks[16]);

void c_world_add_child(CollisionWorld *world, void *child);
void *c_world_remove_object_from_world(void *child);

#endif /* collision_world_h */
