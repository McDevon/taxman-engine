#include "physics_world.h"
#include "physics_body.h"

struct PhysicsWorld {
    GAME_OBJECT_COMPONENT;
    ArrayList *physics_components;
    ArrayList *sweep_list_x;
    TileMap *w_tilemap;
    void *w_callback_context;
    collision_callback_t *trigger_collision;
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

int pbd_compare_left_edge(const void *a, const void *b)
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
}

void world_test_object_collisions(PhysicsWorld *self)
{
    list_sort_insertsort(self->sweep_list_x, pbd_compare_left_edge);
    
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
            
            if (body->collision_mask & other_body->collision_mask) {
                Number top = parent->position.y + body->body_rect.origin.y;
                Number bottom = parent->position.y + body->body_rect.origin.y + body->body_rect.size.height;
                Number other_top = other_parent->position.y + other_body->body_rect.origin.y;
                Number other_bottom = other_parent->position.y + other_body->body_rect.origin.y + other_body->body_rect.size.height;
                if (other_bottom > top && other_top < bottom) {
                    self->trigger_collision(self->w_callback_context, body, other_body);
                }
            }
        }
    }
}

Number world_collision_distance(Tile *tile, PhysicsBody *body, Direction move_direction, Number tile_size, int32_t tile_row_index, Number current_position)
{
    if (tile
        && tile->collision_mask & body->collision_mask
        && tile->collision_directions & (1 << dir_opposite(move_direction)))
    {
        if (dir_positive(move_direction)) {
            Number body_size = dir_horizontal(move_direction) ? body->body_rect.size.width : body->body_rect.size.height;
            return tile_row_index * tile_size - current_position - body_size;
        } else {
            return current_position - (tile_row_index + 1) * tile_size;
        }
    }
    return -nb_one;
}

void world_fixed_update(GameObjectComponent *comp, Number dt_ms)
{
    PhysicsWorld *self = (PhysicsWorld *)comp;
    
    if (self->trigger_collision) {
        world_test_object_collisions(self);
    }

    size_t po_count = list_count(self->physics_components);
    for (size_t i = 0; i < po_count; ++i) {
        PhysicsBody *body = (PhysicsBody *)list_get(self->physics_components, i);
        GameObject *object = comp_get_parent(body);
        
        if (body->gravity_affects) {
            body->velocity.x += nb_mul(self->gravity.x, dt_ms) / 1000;
            body->velocity.y += nb_mul(self->gravity.y, dt_ms) / 1000;
        }
        
        Vector2D control_movement = vec(nb_mul(body->control_movement.x, dt_ms) / 1000,
                                        nb_mul(body->control_movement.y, dt_ms) / 1000);
        Vector2D target_movement = vec_vec_add(body->velocity, control_movement);

        body->collision_dir[dir_left] = False;
        body->collision_dir[dir_right] = False;
        body->collision_dir[dir_up] = False;
        body->collision_dir[dir_down] = False;
        
        if (self->w_tilemap) {
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
            
            Bool tile_collision = False;

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
                    Number collision_distance = world_collision_distance(tile, body, dir, tile_height, tile_y, current_position.y);
                    if (collision_distance >= nb_zero)
                    {
                        target_movement.y = collision_distance * dist_multiplier;
                        body->velocity.y = nb_zero;
                        body->collision_dir[dir] = True;
                        tile_collision = True;
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
                    Number collision_distance = world_collision_distance(tile, body, dir, tile_width, tile_x, current_position.x);
                    if (collision_distance >= nb_zero)
                    {
                        target_movement.x = collision_distance * dist_multiplier;
                        body->velocity.x = nb_zero;
                        body->collision_dir[dir] = True;
                        tile_collision = True;
                        break;
                    }
                }
            }
            if (!tile_collision && tile_x >= 0 && tile_y >= 0) {
                
                Direction dir = target_movement.y > nb_zero ? dir_down : dir_up;
                Number dist_multiplier = target_movement.y > nb_zero ? 1 : -1;
                
                Tile *tile = tilemap_tile_at(self->w_tilemap, tile_x, tile_y);
                Number collision_distance = world_collision_distance(tile, body, dir, tile_height, tile_y, current_position.y);
                if (collision_distance >= nb_zero)
                {
                    target_movement.y = collision_distance * dist_multiplier;
                    body->velocity.y = nb_zero;
                    body->collision_dir[dir] = True;
                }
            }
        }
    
        object->position = vec_vec_add(target_movement, object->position);
    }
}

static GameObjectComponentType PhysicsWorldComponentType = {
    { { "PhysicsWorld", &world_destroy, &world_describe } },
    &world_added,
    &world_start,
    &world_update,
    &world_fixed_update
};

PhysicsWorld *world_create(void *callback_context, collision_callback_t *trigger_collision)
{
    PhysicsWorld *self = (PhysicsWorld *)comp_alloc(sizeof(PhysicsWorld));
    
    self->w_type = &PhysicsWorldComponentType;
    self->physics_components = list_create_with_weak_references();
    self->sweep_list_x = list_create_with_weak_references();
    self->gravity = vec(nb_from_int(0), nb_from_int(18));
    self->trigger_collision = trigger_collision;
    self->w_callback_context = callback_context;
    
    return self;
}

void world_add_child(PhysicsWorld *self, void *child)
{
    go_add_child(self, child);
    
    PhysicsBody *body = (PhysicsBody*)go_get_component(child, &PhysicsBodyComponentType);
    if (body) {
        list_add(self->physics_components, body);
        list_add(self->sweep_list_x, body);
    }
}

void *world_remove_object_from_parent(void *child)
{
    GameObject *parent = go_get_parent(child);
    GameObject *dropped_child = (GameObject *)go_remove_from_parent(child);
    
    if (dropped_child && parent->w_type == &PhysicsWorldComponentType) {
        PhysicsWorld *world = (PhysicsWorld*)parent;
        list_drop_item(world->physics_components, child);
        list_drop_item(world->sweep_list_x, child);
    }
    
    return dropped_child;
}