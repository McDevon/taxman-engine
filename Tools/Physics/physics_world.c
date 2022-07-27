#include "physics_world.h"
#include "physics_body.h"

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
                         uint8_t row16[1])
{
    uint8_t *rows[] = {
        row1, row2, row3, row4, row5, row6, row7, row8,
        row9, row10, row11, row12, row13, row14, row15, row16
    };
    
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16 - i; ++j) {
            int k = 15 - i;
            uint16_t is_set = rows[i][j] > 0;
            masks[j] |= is_set << k;
            masks[k] |= is_set << j;
        }
    }
    
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "uint16_t *masks = { ");
    for (int i = 0; i < 16; ++i) {
        sb_append_string(sb, "0x");
        sb_append_hex(sb, masks[i]);
        if (i < 15) {
            sb_append_string(sb, ", ");
        }
    }
    sb_append_string(sb, " };\n");
    
    sb_debug_log_to_console(sb);
    destroy(sb);
}

void set_collision_masks_8(uint16_t *masks,
                           uint8_t row9[8],
                           uint8_t row10[7],
                           uint8_t row11[6],
                           uint8_t row12[5],
                           uint8_t row13[4],
                           uint8_t row14[3],
                           uint8_t row15[2],
                           uint8_t row16[1])
{
    uint8_t row1[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t row2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t row3[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t row4[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t row5[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t row6[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t row7[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t row8[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    
    uint8_t *rows[] = {
        row1, row2, row3, row4, row5, row6, row7, row8,
        row9, row10, row11, row12, row13, row14, row15, row16
    };
    
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16 - i; ++j) {
            int k = 15 - i;
            uint16_t is_set = rows[i][j] > 0;
            masks[j] |= is_set << k;
            masks[k] |= is_set << j;
        }
    }
    
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "uint16_t *masks = { ");
    for (int i = 0; i < 16; ++i) {
        sb_append_string(sb, "0x");
        sb_append_hex(sb, masks[i]);
        if (i < 15) {
            sb_append_string(sb, ", ");
        }
    }
    sb_append_string(sb, " };\n");
    
    sb_debug_log_to_console(sb);
    destroy(sb);
}

struct PhysicsWorld {
    GAME_OBJECT_COMPONENT;
    ArrayList *physics_components;
    ArrayList *sweep_list_x;
    TileMap *w_tilemap;
    void *w_callback_context;
    collision_callback_t *trigger_collision;
    uint16_t collision_masks[16];
    Vector2D gravity;
};

void world_destroy(void *comp)
{
    PhysicsWorld *self = (PhysicsWorld *)comp;
    destroy(self->physics_components);
    destroy(self->sweep_list_x);
    comp_destroy(comp);
}

char *world_describe(void *comp)
{
    return comp_describe(comp);
}

void world_added(GameObjectComponent *comp)
{
    
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
                list_add(self->sweep_list_x, component);
                break;
            }
        }
    }
}

void world_update(GameObjectComponent *comp, Number dt_ms)
{
}

/*int pbd_compare_left_edge(const void *a, const void *b)
{
    PhysicsBody *bd_a = *(PhysicsBody**)a;
    PhysicsBody *bd_b = *(PhysicsBody**)b;
    GameObject *parent_a = comp_get_parent(bd_a);
    GameObject *parent_b = comp_get_parent(bd_b);
    
    Number diff = parent_a->position.x + bd_a->body_rect.origin.x - parent_b->position.x + bd_b->body_rect.origin.x;
    
    if (diff < nb_zero) {
        return list_sorted_ascending;
    } else if (diff > nb_zero) {
        return list_sorted_descending;
    } else {
        return list_sorted_same;
    }
}*/

void world_test_object_collisions(PhysicsWorld *self)
{
    /*list_sort_insertsort(self->sweep_list_x, pbd_compare_left_edge);
    
    size_t count = list_count(self->sweep_list_x);
    for (size_t i = 0; i < count; ++i) {
        PhysicsBody *body = list_get(self->sweep_list_x, i);
        GameObject *parent = comp_get_parent(body);
        
        Number right_edge = parent->position.x + body->body_rect.origin.x + body->body_rect.size.width;

        for (size_t k = i + 1; k < count; ++k) {
            PhysicsBody *other_body = list_get(self->sweep_list_x, k);
            GameObject *other_parent = comp_get_parent(other_body);
            
            if (other_parent->position.x + other_body->body_rect.origin.x >= right_edge) {
                break;
            }
            
            if (self->collision_masks[body->collision_layer] & (1 << other_body->collision_layer)) {
                Number top = parent->position.y + body->body_rect.origin.y;
                Number bottom = parent->position.y + body->body_rect.origin.y + body->body_rect.size.height;
                Number other_top = other_parent->position.y + other_body->body_rect.origin.y;
                Number other_bottom = other_parent->position.y + other_body->body_rect.origin.y + other_body->body_rect.size.height;
                if (other_bottom > top && other_top < bottom) {
                    self->trigger_collision(self->w_callback_context, body, other_body);
                }
            }
        }
    }*/
}

Number world_collision_distance(PhysicsWorld *self, Tile *tile, PhysicsBody *body, Direction move_direction, Number tile_size, int32_t tile_row_index, Number current_position)
{
    /*if (tile
        && self->collision_masks[tile->collision_layer] & (1 << body->collision_layer)
        && tile->collision_directions & (1 << dir_opposite(move_direction)))
    {
        if (dir_positive(move_direction)) {
            Number body_size = dir_horizontal(move_direction) ? body->body_rect.size.width : body->body_rect.size.height;
            return tile_row_index * tile_size - current_position - body_size;
        } else {
            return current_position - (tile_row_index + 1) * tile_size;
        }
    }*/
    return -nb_one;
}

void world_fixed_update(GameObjectComponent *comp, Number dt_ms)
{
    /*
#ifdef ENABLE_PROFILER
    profiler_start_segment("Physics world");
#endif
    PhysicsWorld *self = (PhysicsWorld *)comp;
    
#ifdef ENABLE_PROFILER
    profiler_start_segment("Object collision");
#endif
    if (self->trigger_collision) {
        world_test_object_collisions(self);
    }
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
    
    size_t po_count = list_count(self->physics_components);
    for (size_t i = 0; i < po_count; ++i) {
#ifdef ENABLE_PROFILER
        profiler_start_segment("Object movement");
#endif
        PhysicsBody *body = (PhysicsBody *)list_get(self->physics_components, i);
        GameObject *object = comp_get_parent(body);
        
        if (body->gravity_affects) {
            body->velocity.x += nb_mul(self->gravity.x, dt_ms) / 1000;
            body->velocity.y += nb_mul(self->gravity.y, dt_ms) / 1000;
        }
        
        Vector2D control_movement = vec(nb_mul(body->control_movement.x, dt_ms) / 1000,
                                        nb_mul(body->control_movement.y, dt_ms) / 1000);
        Vector2D velocity_movement = vec(nb_mul(body->velocity.x, dt_ms) / 1000,
                                         nb_mul(body->velocity.y, dt_ms) / 1000);
        Vector2D target_movement = vec_vec_add(velocity_movement, control_movement);
        
#ifdef ENABLE_PROFILER
        profiler_end_segment();
#endif
        
        if (self->w_tilemap) {
#ifdef ENABLE_PROFILER
            profiler_start_segment("Tilemap collision");
#endif
            
            body->collision_dir[dir_left] = false;
            body->collision_dir[dir_right] = false;
            body->collision_dir[dir_up] = false;
            body->collision_dir[dir_down] = false;

            Vector2D target_node_position = vec_vec_add(target_movement, object->position);
            Vector2D current_position = vec(object->position.x + body->body_rect.origin.x,
                                            object->position.y + body->body_rect.origin.y);
            Vector2D target_position = vec(target_node_position.x + body->body_rect.origin.x,
                                           target_node_position.y + body->body_rect.origin.y);
            Number tile_width = self->w_tilemap->tile_size.width;
            Number tile_height = self->w_tilemap->tile_size.height;
            
            int32_t tile_x = -1, tile_y = -1;
            Number x_current = nb_zero, x_target = nb_zero;
            Number y_current = nb_zero, y_target = nb_zero;
            
            bool tile_collision = false;

            if (target_movement.x > nb_zero) {
                x_current = current_position.x + body->body_rect.size.width - 1;
                x_target = target_position.x + body->body_rect.size.width;
                int32_t tile_current = nb_to_int(nb_div(x_current, tile_width));
                int32_t tile_target = nb_to_int(nb_div(x_target, tile_width));
                if (tile_target > tile_current) {
                    tile_x = tile_target;
                }
            } else if (target_movement.x < nb_zero) {
                x_current = current_position.x + 1;
                x_target = target_position.x;
                int32_t tile_current = nb_to_int(nb_div(x_current, tile_width));
                int32_t tile_target = nb_to_int(nb_div(x_target, tile_width));
                if (tile_target < tile_current) {
                    tile_x = tile_target;
                }
            }
            if (target_movement.y > nb_zero) {
                y_current = current_position.y + body->body_rect.size.height - 1;
                y_target = target_position.y + body->body_rect.size.height;
                int32_t tile_current = nb_to_int(nb_div(y_current, tile_width));
                int32_t tile_target = nb_to_int(nb_div(y_target, tile_width));
                if (tile_target > tile_current) {
                    tile_y = tile_target;
                }
            } else if (target_movement.y < nb_zero) {
                y_current = current_position.y + 1;
                y_target = target_position.y;
                int32_t tile_current = nb_to_int(nb_div(y_current, tile_width));
                int32_t tile_target = nb_to_int(nb_div(y_target, tile_width));
                if (tile_target < tile_current) {
                    tile_y = tile_target;
                }
            }

            if (tile_y >= 0) {
                Number left_edge = current_position.x + 1;
                Number right_edge = current_position.x + body->body_rect.size.width - 1;
                int32_t x_start = nb_to_int(nb_div(left_edge, tile_width));
                int32_t x_end = nb_to_int(nb_div(right_edge, tile_width));
                
                Direction dir = target_movement.y > nb_zero ? dir_down : dir_up;
                Number dist_multiplier = target_movement.y > nb_zero ? 1 : -1;

                for (int32_t x = x_start; x <= x_end; ++x) {
                    Tile *tile = tilemap_tile_at(self->w_tilemap, x, tile_y);
                    Number collision_distance = world_collision_distance(self, tile, body, dir, tile_height, tile_y, current_position.y);
                    if (collision_distance >= nb_zero)
                    {
                        target_movement.y = collision_distance * dist_multiplier;
                        body->velocity.y = nb_zero;
                        body->collision_dir[dir] = true;
                        tile_collision = true;
                        break;
                    }
                }
            }
            if (tile_x >= 0) {
                Number top_edge = current_position.y + 1;
                Number bottom_edge = current_position.y + body->body_rect.size.height - 1;
                int32_t y_start = nb_to_int(nb_div(top_edge, tile_height));
                int32_t y_end = nb_to_int(nb_div(bottom_edge, tile_height));
                
                Direction dir = target_movement.x > nb_zero ? dir_right : dir_left;
                Number dist_multiplier = target_movement.x > nb_zero ? 1 : -1;
                
                for (int32_t y = y_start; y <= y_end; ++y) {
                    Tile *tile = tilemap_tile_at(self->w_tilemap, tile_x, y);
                    Number collision_distance = world_collision_distance(self, tile, body, dir, tile_width, tile_x, current_position.x);
                    if (collision_distance >= nb_zero)
                    {
                        target_movement.x = collision_distance * dist_multiplier;
                        body->velocity.x = nb_zero;
                        body->collision_dir[dir] = true;
                        tile_collision = true;
                        break;
                    }
                }
            }
            if (!tile_collision && tile_x >= 0 && tile_y >= 0) {
                
                Direction dir = target_movement.y > nb_zero ? dir_down : dir_up;
                Number dist_multiplier = target_movement.y > nb_zero ? 1 : -1;
                
                Tile *tile = tilemap_tile_at(self->w_tilemap, tile_x, tile_y);
                Number collision_distance = world_collision_distance(self, tile, body, dir, tile_height, tile_y, current_position.y);
                if (collision_distance >= nb_zero)
                {
                    target_movement.y = collision_distance * dist_multiplier;
                    body->velocity.y = nb_zero;
                    body->collision_dir[dir] = true;
                }
            }
#ifdef ENABLE_PROFILER
            profiler_end_segment();
#endif
        }
    
        object->position = vec_vec_add(target_movement, object->position);
    }
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
     */
}

GameObjectComponentType PhysicsWorldComponentType = {
    { { "PhysicsWorld", &world_destroy, &world_describe } },
    &world_added,
    NULL,
    &world_start,
    &world_update,
    &world_fixed_update
};

PhysicsWorld *world_create(void *callback_context, collision_callback_t *trigger_collision, uint16_t collision_masks[16])
{
    PhysicsWorld *self = (PhysicsWorld *)comp_alloc(sizeof(PhysicsWorld));
    
    self->w_type = &PhysicsWorldComponentType;
    self->physics_components = list_create_with_weak_references();
    self->sweep_list_x = list_create_with_weak_references();
    self->gravity = vec(nb_from_int(0), nb_from_int(450));
    self->trigger_collision = trigger_collision;
    self->w_callback_context = callback_context;
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
        list_add(self->sweep_list_x, body);
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
    list_drop_item(world->sweep_list_x, body);
    
    return child;
}

bool directions_contains_direction(DirectionTable directions, Direction direction)
{
    return ((direction == dir_up && directions.up)
            || (direction == dir_down && directions.down)
            || (direction == dir_left && directions.left)
            || (direction == dir_right && directions.right));
}

PhysicsBody * world_pbd_dynamic_collides_static(PhysicsWorld *world, PhysicsBody *physics_body, Vector2D position, Direction moving_direction)
{
    return NULL;
}

bool world_pbd_dynamic_collides_tile(PhysicsWorld *world, PhysicsBody *physics_body, Vector2D new_position, Direction moving_direction)
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
    
    int32_t x_start, x_end, y_start, y_end;
    
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
        Number next = new_position.y + physics_body->size.height;
        
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
        
        y_start = nb_to_int(nb_floor(nb_div(pbd_top(physics_body), tile_width)));
        y_end = nb_to_int(nb_floor(nb_div(pbd_bottom(physics_body), tile_width)));
    } else if (moving_direction == dir_right) {
        Number current = pbd_right(physics_body);
        Number next = new_position.x + physics_body->size.width;
        
        x_start = nb_to_int(nb_floor(nb_div(current, tile_width)));
        x_end = nb_to_int(nb_floor(nb_div(next, tile_width)));
        
        if (x_start == x_end) {
            return false;
        }
        
        ++x_start;
        
        y_start = nb_to_int(nb_floor(nb_div(pbd_top(physics_body), tile_width)));
        y_end = nb_to_int(nb_floor(nb_div(pbd_bottom(physics_body), tile_width)));
    } else {
        x_start = -1;
        x_end = -1;
        y_start = -1;
        y_end = -1;
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
            if (world_pbd_dynamic_collides_tile(world, physics_body, next_position, moving_direction)) {
                if (callback) {
                    callback(physics_body, NULL, moving_direction, collision_context);
                }
                break;
            }
            PhysicsBody *collided_static = world_pbd_dynamic_collides_static(world, physics_body, next_position, moving_direction);
            if (!collided_static) {
                physics_body->position.x += sign;
                move -= sign;
            } else {
                if (callback) {
                    callback(physics_body, collided_static, moving_direction, collision_context);
                }
                break;
            }
        }
    }
}

void world_pbd_move_dynamic_y(PhysicsWorld *world, PhysicsBody *physics_body, Number movement, pbd_collision_callback_t *callback, void *collision_context)
{
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
            if (world_pbd_dynamic_collides_tile(world, physics_body, next_position, moving_direction)) {
                if (callback) {
                    callback(physics_body, NULL, sign > nb_zero ? dir_down : dir_up, collision_context);
                }
                break;
            }
            PhysicsBody *collided_static = world_pbd_dynamic_collides_static(world, physics_body, next_position, moving_direction);
            if (collided_static) {
                if (callback) {
                    callback(physics_body, collided_static, moving_direction, collision_context);
                }
                break;
            } else {
                physics_body->position.y += sign;
                move -= sign;
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
        static_body->remainder_movement.x -= move_x;
        static_body->position.x += move_x;
        for_each_begin(PhysicsBody *, dynamic_body, world->physics_components) {
            if (!dynamic_body->dynamic || (world->collision_masks[static_body->collision_layer] & (1 << dynamic_body->collision_layer)) == 0) {
                continue;
            }
            
            if (pbd_overlap(static_body, dynamic_body)) {
                if (move_x > nb_zero && static_body->collision_directions.right && dynamic_body->collision_directions.left) {
                    world_pbd_move_dynamic_x(world, dynamic_body, pbd_right(static_body) - pbd_left(dynamic_body), &pbd_crush, NULL);
                } else if (move_x < nb_zero && static_body->collision_directions.left && dynamic_body->collision_directions.right){
                    world_pbd_move_dynamic_x(world, dynamic_body, pbd_left(static_body) - pbd_right(dynamic_body), &pbd_crush, NULL);
                }
            } else if (dynamic_body->w_mount == static_body) {
                world_pbd_move_dynamic_x(world, dynamic_body, move_x, NULL, NULL);
            }
        }
        for_each_end;
    }
    if (move_y != nb_zero) {
        static_body->remainder_movement.y -= move_y;
        static_body->position.y += move_y;
        for_each_begin(PhysicsBody *, dynamic_body, world->physics_components) {
            if (!dynamic_body->dynamic || (world->collision_masks[static_body->collision_layer] & (1 << dynamic_body->collision_layer)) == 0) {
                continue;
            }

            if (pbd_overlap(static_body, dynamic_body)) {
                if (move_y > nb_zero && static_body->collision_directions.down && dynamic_body->collision_directions.up) {
                    world_pbd_move_dynamic_y(world, dynamic_body, pbd_bottom(static_body) - pbd_top(dynamic_body), &pbd_crush, NULL);
                } else if (move_y < nb_zero && static_body->collision_directions.up && dynamic_body->collision_directions.down) {
                    world_pbd_move_dynamic_y(world, dynamic_body, pbd_top(static_body) - pbd_bottom(dynamic_body), &pbd_crush, NULL);
                }
            } else if (dynamic_body->w_mount == static_body) {
                world_pbd_move_dynamic_y(world, dynamic_body, move_y, NULL, NULL);
            }
        }
        for_each_end;
    }
    
    static_body->collision_directions = collisions;
}
