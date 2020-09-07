#ifndef physics_world_h
#define physics_world_h

#include "engine.h"
#include "tilemap.h"

struct PhysicsBody;

typedef struct PhysicsWorld PhysicsWorld;
typedef void (collision_callback_t)(void *context, struct PhysicsBody *obj_a, struct PhysicsBody *obj_b);

PhysicsWorld *world_create(void *callback_context, collision_callback_t *trigger_collision);

void world_add_child(PhysicsWorld *world, void *child);
void *world_remove_object_from_parent(void *child);

#endif /* physics_world_h */
