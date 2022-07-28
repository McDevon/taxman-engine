#ifndef physics_object_component_h
#define physics_object_component_h

#include "engine.h"
#include "physics_world.h"

struct PhysicsBody;

typedef struct PhysicsBody {
    GAME_OBJECT_COMPONENT;
    PhysicsWorld *w_world;
    struct PhysicsBody *w_mount;
    pbd_collision_callback_t *crush_override_callback;
    pbd_collision_callback_t *push_callback;
    void *w_callback_context;
    Vector2D position;
    Vector2D object_offset;
    Size2D size;
    Vector2D remainder_movement;
    uint8_t collision_layer;
    DirectionTable collision_directions;
    bool dynamic;
    bool trigger;
} PhysicsBody;

extern GameObjectComponentType PhysicsBodyComponentType;

PhysicsBody *pbd_create(void);

void pbd_set_body_rect_to_parent(PhysicsBody *physics_body);
void pbd_set_position_to_parent(PhysicsBody *physics_body);

void pbd_move_dynamic(PhysicsBody *physics_body, Vector2D movement, pbd_collision_callback_t callback, void *collision_context);
void pbd_move_static(PhysicsBody *physics_body, Vector2D movement);

void pbd_crush(PhysicsBody *physics_body, PhysicsBody *crushing_body, Direction direction, void *collision_context);
void pbd_pushed(PhysicsBody *physics_body, PhysicsBody *pushing_body, Direction moving_direction);

Number pbd_left(PhysicsBody *physics_body);
Number pbd_right(PhysicsBody *physics_body);
Number pbd_top(PhysicsBody *physics_body);
Number pbd_bottom(PhysicsBody *physics_body);

bool pbd_overlap(PhysicsBody *pbd_a, PhysicsBody *pbd_b);
bool pbd_overlap_in_position(PhysicsBody *pbd_a, PhysicsBody *pbd_b, Vector2D pbd_a_position);
bool pbd_vertical_overlap(PhysicsBody *pbd_a, PhysicsBody *pbd_b);

#endif /* physics_object_component_h */
