#include "physics_world.h"
#include "physics_body.h"

struct PhysicsWorld {
    GAME_OBJECT_COMPONENT;
    ArrayList *physics_components;
    TileMap *w_tilemap;
    uint16_t collision_masks[16];
};

void world_destroy(void *comp)
{
    PhysicsWorld *self = (PhysicsWorld *)comp;
    destroy(self->physics_components);
    comp_destroy(comp);
}

char *world_describe(void *comp)
{
    return comp_describe(comp);
}

void world_start(GameObjectComponent *comp)
{
    PhysicsWorld *self = (PhysicsWorld *)comp;
    
    GameObject *parent = comp_get_parent(self);
    if (parent->w_type == &TileMapType) {
        self->w_tilemap = (TileMap *)parent;
    }
    
    ArrayList *children = go_get_children(parent);
    size_t child_count = list_count(children);
    for (size_t i = 0; i < child_count; ++i) {
        GameObject *child = list_get(children, i);
        ArrayList *components = go_get_components(child);
        size_t comp_count = list_count(components);
        for (size_t k = 0; k < comp_count; ++k) {
            GameObjectComponent *component = list_get(components, k);
            if (component->w_type == &PhysicsBodyComponentType) {
                list_add(self->physics_components, component);
                break;
            }
        }
    }
}

GameObjectComponentType PhysicsWorldComponentType = {
    { { "PhysicsWorld", &world_destroy, &world_describe } },
    NULL,
    NULL,
    &world_start,
    NULL,
    NULL
};

PhysicsWorld *world_create(uint16_t collision_masks[16])
{
    PhysicsWorld *self = (PhysicsWorld *)comp_alloc(sizeof(PhysicsWorld));
    
    self->w_type = &PhysicsWorldComponentType;
    self->physics_components = list_create_with_weak_references();
    for (int i = 0; i < 16; ++i) {
        self->collision_masks[i] = collision_masks[i];
    }
    
    return self;
}

void world_add_child(PhysicsWorld *self, void *child)
{
    if (!go_get_parent(child)) {
        go_add_child(self, child);
    }
    
    PhysicsBody *body = (PhysicsBody*)go_get_component(child, &PhysicsBodyComponentType);
    if (body && !list_contains(self->physics_components, body)) {
        list_add(self->physics_components, body);
    }
    body->w_world = self;
}

void *world_remove_object_from_world(void *child)
{
    PhysicsBody *body = (PhysicsBody*)go_get_component(child, &PhysicsBodyComponentType);
    if (!body) {
        LOG_ERROR("Trying to remove object from world: does not have body");
        return NULL;
    }
    
    GameObject *parent = go_get_parent(child);
    
    if (!parent) {
        LOG_ERROR("Trying to remove object from world: does not have parent");
        return NULL;
    }
    
    PhysicsWorld *world = (PhysicsWorld*)go_get_component(parent, &PhysicsWorldComponentType);
    
    if (!world) {
        LOG_ERROR("Trying to remove object from world: parent does not have world");
        return NULL;
    }
    
    list_drop_item(world->physics_components, body);
    
    return child;
}

bool directions_contains_direction(DirectionTable directions, Direction direction)
{
    return ((direction == dir_up && directions.up)
            || (direction == dir_down && directions.down)
            || (direction == dir_left && directions.left)
            || (direction == dir_right && directions.right));
}

PhysicsBody *world_pbd_collides_static_if_moves_to(PhysicsWorld *world, PhysicsBody *physics_body, Vector2D position, Direction moving_direction)
{
    if (!directions_contains_direction(physics_body->collision_directions, moving_direction)) {
        return NULL;
    }
    for_each_begin(PhysicsBody *, other_body, world->physics_components) {
        if (physics_body == other_body) {
            continue;
        }
        if (other_body->dynamic || (world->collision_masks[physics_body->collision_layer] & (1 << other_body->collision_layer)) == 0) {
            continue;
        }
        
        if (pbd_overlap_in_position(physics_body, other_body, position) && !pbd_overlap(physics_body, other_body)) {
            if (directions_contains_direction(other_body->collision_directions, dir_opposite(moving_direction))) {
                return other_body;
            }
        }
    }
    for_each_end
    
    return NULL;
}

bool world_pbd_collides_tile_if_moves_to(PhysicsWorld *world, PhysicsBody *physics_body, Vector2D new_position, Direction moving_direction)
{
    if (!world->w_tilemap) {
        return false;
    }
    if (!directions_contains_direction(physics_body->collision_directions, moving_direction)) {
        return false;
    }
    
    TileMap *tilemap = world->w_tilemap;
    
    Number tile_width = tilemap->tile_size.width;
    Number tile_height = tilemap->tile_size.height;
    
    int32_t x_start = -1, x_end = -1, y_start = -1, y_end = -1;
    
    if (moving_direction == dir_up) {
        Number current = pbd_top(physics_body);
        Number next = new_position.y;
        
        y_start = nb_to_int(nb_floor(nb_div(next, tile_height)));
        y_end = nb_to_int(nb_floor(nb_div(current, tile_height)));
        
        if (y_start == y_end) {
            return false;
        }
        
        --y_end;
        
        x_start = nb_to_int(nb_floor(nb_div(pbd_left(physics_body), tile_width)));
        x_end = nb_to_int(nb_floor(nb_div(pbd_right(physics_body), tile_width)));
    } else if (moving_direction == dir_down) {
        Number current = pbd_bottom(physics_body);
        Number next = new_position.y + physics_body->size.height - nb_one;
        
        y_start = nb_to_int(nb_floor(nb_div(current, tile_height)));
        y_end = nb_to_int(nb_floor(nb_div(next, tile_height)));
        
        if (y_start == y_end) {
            return false;
        }
        
        ++y_start;
        
        x_start = nb_to_int(nb_floor(nb_div(pbd_left(physics_body), tile_width)));
        x_end = nb_to_int(nb_floor(nb_div(pbd_right(physics_body), tile_width)));
    } else if (moving_direction == dir_left) {
        Number current = pbd_left(physics_body);
        Number next = new_position.x;
        
        x_start = nb_to_int(nb_floor(nb_div(next, tile_width)));
        x_end = nb_to_int(nb_floor(nb_div(current, tile_width)));
        
        if (x_start == x_end) {
            return false;
        }
        
        --x_end;
        
        y_start = nb_to_int(nb_floor(nb_div(pbd_top(physics_body), tile_height)));
        y_end = nb_to_int(nb_floor(nb_div(pbd_bottom(physics_body), tile_height)));
    } else if (moving_direction == dir_right) {
        Number current = pbd_right(physics_body);
        Number next = new_position.x + physics_body->size.width - nb_one;
        
        x_start = nb_to_int(nb_floor(nb_div(current, tile_width)));
        x_end = nb_to_int(nb_floor(nb_div(next, tile_width)));
        
        if (x_start == x_end) {
            return false;
        }
        
        ++x_start;
        
        y_start = nb_to_int(nb_floor(nb_div(pbd_top(physics_body), tile_height)));
        y_end = nb_to_int(nb_floor(nb_div(pbd_bottom(physics_body), tile_height)));
    }
    
    for (int32_t y = y_start; y <= y_end; ++y) {
        for (int32_t x = x_start; x <= x_end; ++x) {
            Tile *tile = tilemap_tile_at(tilemap, x, y);
            if (!tile || ((world->collision_masks[tile->collision_layer] & (1 << physics_body->collision_layer)) == 0)) {
                continue;
            }
            if (directions_contains_direction(tile->collision_directions, dir_opposite(moving_direction))) {
                return true;                
            }
        }
    }
    
    return false;
}

void world_pbd_move_dynamic_x(PhysicsWorld *world, PhysicsBody *physics_body, Number movement, pbd_collision_callback_t *callback, void *collision_context)
{
    if (!physics_body->active) {
        return;
    }
    if (!physics_body->dynamic) {
        LOG_WARNING("Trying to move a static physics body with world_pbd_move_dynamic_x()");
        return;
    }
    
    physics_body->remainder_movement.x += movement;
    Number move = nb_round(physics_body->remainder_movement.x);
    if (move != nb_zero) {
        physics_body->remainder_movement.x -= move;
        Number sign = nb_sign(move);
         while (move != nb_zero) {
            Direction moving_direction = sign > nb_zero ? dir_right : dir_left;
            Vector2D next_position = vec(physics_body->position.x + sign, physics_body->position.y);
            if (world_pbd_collides_tile_if_moves_to(world, physics_body, next_position, moving_direction)) {
                if (callback) {
                    callback(physics_body, NULL, moving_direction, collision_context);
                }
                break;
            }
            PhysicsBody *collided_static = world_pbd_collides_static_if_moves_to(world, physics_body, next_position, moving_direction);
            if (!collided_static || collided_static->trigger) {
                physics_body->position.x += sign;
                move -= sign;
            }
            if (collided_static) {
                if (callback) {
                    callback(physics_body, collided_static, moving_direction, collision_context);
                }
                if (!collided_static->trigger) {
                    break;
                }
            }
        }
    }
}

void world_pbd_move_dynamic_y(PhysicsWorld *world, PhysicsBody *physics_body, Number movement, pbd_collision_callback_t *callback, void *collision_context)
{
    if (!physics_body->active) {
        return;
    }
    if (!physics_body->dynamic) {
        LOG_WARNING("Trying to move a static physics body with world_pbd_move_dynamic_y()");
        return;
    }
    
    physics_body->remainder_movement.y += movement;
    Number move = nb_round(physics_body->remainder_movement.y);
    if (move != nb_zero) {
        physics_body->remainder_movement.y -= move;
        Number sign = nb_sign(move);
        while (move != nb_zero) {
            Direction moving_direction = sign > nb_zero ? dir_down : dir_up;
            Vector2D next_position = vec(physics_body->position.x, physics_body->position.y + sign);
            if (world_pbd_collides_tile_if_moves_to(world, physics_body, next_position, moving_direction)) {
                if (callback) {
                    callback(physics_body, NULL, sign > nb_zero ? dir_down : dir_up, collision_context);
                }
                break;
            }
            PhysicsBody *collided_static = world_pbd_collides_static_if_moves_to(world, physics_body, next_position, moving_direction);
            if (!collided_static || collided_static->trigger) {
                physics_body->position.y += sign;
                move -= sign;
            }
            if (collided_static) {
                if (callback) {
                    callback(physics_body, collided_static, moving_direction, collision_context);
                }
                if (!collided_static->trigger) {
                    break;                    
                }
            }
        }
    }
}

void world_pbd_move_dynamic(PhysicsWorld *world, PhysicsBody *physics_body, Vector2D movement, pbd_collision_callback_t *callback, void *collision_context)
{
    if (movement.x != nb_zero) {
        world_pbd_move_dynamic_x(world, physics_body, movement.x, callback, collision_context);
    }
    if (movement.y != nb_zero) {
        world_pbd_move_dynamic_y(world, physics_body, movement.y, callback, collision_context);
    }
}

void world_copy_collisions(bool collisions_from[4], bool *collisions_to)
{
    collisions_to[0] = collisions_from[0];
    collisions_to[1] = collisions_from[1];
    collisions_to[2] = collisions_from[2];
    collisions_to[3] = collisions_from[3];
}

void world_clear_collisions(bool *collisions)
{
    collisions[0] = false;
    collisions[1] = false;
    collisions[2] = false;
    collisions[3] = false;
}

void world_pbd_move_static(PhysicsWorld *world, PhysicsBody *static_body, Vector2D movement)
{
    if (!static_body->active) {
        return;
    }
    if (static_body->dynamic) {
        LOG_WARNING("Trying to move a dynamic physics body with world_move_static()");
        return;
    }
    
    static_body->remainder_movement = vec_vec_add(static_body->remainder_movement, movement);
    
    Number move_x = nb_round(static_body->remainder_movement.x);
    Number move_y = nb_round(static_body->remainder_movement.y);
    
    DirectionTable collisions = static_body->collision_directions;
    static_body->collision_directions = directions_none;
    
    if (move_x != nb_zero) {
        Number previous_x = static_body->position.x;
        static_body->remainder_movement.x -= move_x;
        static_body->position.x += move_x;
        for_each_begin(PhysicsBody *, dynamic_body, world->physics_components) {
            if (!dynamic_body->dynamic || (world->collision_masks[static_body->collision_layer] & (1 << dynamic_body->collision_layer)) == 0) {
                continue;
            }

            bool overlap = pbd_overlap(static_body, dynamic_body) && !pbd_overlap_in_position(static_body, dynamic_body, vec(previous_x, static_body->position.y));
            if (overlap && move_x > nb_zero && collisions.right && dynamic_body->collision_directions.left) {
                world_pbd_move_dynamic_x(world, dynamic_body, pbd_right(static_body) - pbd_left(dynamic_body) + nb_one, &pbd_crush, NULL);
                pbd_pushed(dynamic_body, static_body, dir_right);
            } else if (overlap && move_x < nb_zero && collisions.left && dynamic_body->collision_directions.right){
                world_pbd_move_dynamic_x(world, dynamic_body, pbd_left(static_body) - pbd_right(dynamic_body) - nb_one, &pbd_crush, NULL);
                pbd_pushed(dynamic_body, static_body, dir_left);
            } else if (dynamic_body->w_mount == static_body) {
                world_pbd_move_dynamic_x(world, dynamic_body, move_x, NULL, NULL);
            }
        }
        for_each_end;
    }
    if (move_y != nb_zero) {
        Number previous_y = static_body->position.y;
        static_body->remainder_movement.y -= move_y;
        static_body->position.y += move_y;
        for_each_begin(PhysicsBody *, dynamic_body, world->physics_components) {
            if (!dynamic_body->dynamic || (world->collision_masks[static_body->collision_layer] & (1 << dynamic_body->collision_layer)) == 0) {
                continue;
            }
            
            bool overlap = pbd_overlap(static_body, dynamic_body) && !pbd_overlap_in_position(static_body, dynamic_body, vec(static_body->position.x, previous_y));
            if (overlap && move_y > nb_zero && collisions.down && dynamic_body->collision_directions.up) {
                world_pbd_move_dynamic_y(world, dynamic_body, pbd_bottom(static_body) - pbd_top(dynamic_body) + nb_one, &pbd_crush, NULL);
                pbd_pushed(dynamic_body, static_body, dir_down);
            } else if (overlap && move_y < nb_zero && collisions.up && dynamic_body->collision_directions.down) {
                world_pbd_move_dynamic_y(world, dynamic_body, pbd_top(static_body) - pbd_bottom(dynamic_body) - nb_one, &pbd_crush, NULL);
                pbd_pushed(dynamic_body, static_body, dir_up);
            } else if (dynamic_body->w_mount == static_body) {
                world_pbd_move_dynamic_y(world, dynamic_body, move_y, NULL, NULL);
            }
        }
        for_each_end;
    }
    
    static_body->collision_directions = collisions;
}
