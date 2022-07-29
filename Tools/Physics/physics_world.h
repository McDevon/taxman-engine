#ifndef physics_world_h
#define physics_world_h

#include "engine.h"
#include "tilemap.h"

struct PhysicsBody;
extern GameObjectComponentType PhysicsWorldComponentType;

/**
 Physics World is a tool for creating easy platformer and top-down physics environments.
 
 Features:
 - AABB colliders
 - Collision matrix
 - Dynamic objects cannot enter static objects
 - Support for TileMap
 - Moving static objects push dynamic objects out of the way and can be used for moving platforms
 - Collision directions can be set for objects and tiles, allowing one-directional walls and platforms
 */
typedef struct PhysicsWorld PhysicsWorld;
typedef void (pbd_collision_callback_t)(struct PhysicsBody *obj_a, struct PhysicsBody *obj_b, Direction direction, void *context);

PhysicsWorld *world_create(uint16_t collision_masks[16]);

void world_add_child(PhysicsWorld *world, void *child);
void *world_remove_object_from_world(void *child);

bool world_pbd_collides_tile_if_moves_to(PhysicsWorld *world, struct PhysicsBody *physics_body, Vector2D new_position, Direction moving_direction);
struct PhysicsBody *world_pbd_collides_static_if_moves_to(PhysicsWorld *world, struct PhysicsBody *physics_body, Vector2D new_position, Direction moving_direction);

void world_pbd_move_dynamic(PhysicsWorld *world, struct PhysicsBody *physics_body, Vector2D movement, pbd_collision_callback_t *callback, void *collision_context);
void world_pbd_move_static(PhysicsWorld *world, struct PhysicsBody *physics_body, Vector2D movement);

#endif /* physics_world_h */
