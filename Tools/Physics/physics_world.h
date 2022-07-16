#ifndef physics_world_h
#define physics_world_h

#include "engine.h"
#include "tilemap.h"

struct PhysicsBody;
extern GameObjectComponentType PhysicsWorldComponentType;

typedef struct PhysicsWorld PhysicsWorld;
typedef void (collision_callback_t)(void *context, struct PhysicsBody *obj_a, struct PhysicsBody *obj_b);

#define empty_collision_masks { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

void set_collision_masks(uint16_t *masks,
                         uint8_t row1[16],
                         uint8_t row2[15],
                         uint8_t row3[14],
                         uint8_t row4[13],
                         uint8_t row5[12],
                         uint8_t row6[11],
                         uint8_t row7[10],
                         uint8_t row8[9],
                         uint8_t row9[8],
                         uint8_t row10[7],
                         uint8_t row11[6],
                         uint8_t row12[5],
                         uint8_t row13[4],
                         uint8_t row14[3],
                         uint8_t row15[2],
                         uint8_t row16[1]);

PhysicsWorld *world_create(void *callback_context, collision_callback_t *trigger_collision, uint16_t collision_masks[16]);

void world_add_child(PhysicsWorld *world, void *child);
void *world_remove_object_from_world(void *child);

#endif /* physics_world_h */
