#include "collision_world.h"
#include "collision_body.h"

struct CollisionWorld {
    GAME_OBJECT_COMPONENT;
    ArrayList *collision_components;
    ArrayList *sweep_list_x;
    void *w_callback_context;
    collision_world_callback_t *collision_callback;
    uint16_t collision_masks[16];
};

void c_world_destroy(void *comp)
{
    CollisionWorld *self = (CollisionWorld *)comp;
    destroy(self->collision_components);
    destroy(self->sweep_list_x);
    comp_destroy(comp);
}

char *c_world_describe(void *comp)
{
    return comp_describe(comp);
}

void c_world_start(GameObjectComponent *comp)
{
    CollisionWorld *self = (CollisionWorld *)comp;
    
    GameObject *parent = comp_get_parent(self);
    ArrayList *children = go_get_children(parent);
    size_t child_count = list_count(children);
    for (size_t i = 0; i < child_count; ++i) {
        GameObject *child = list_get(children, i);
        ArrayList *components = go_get_components(child);
        size_t comp_count = list_count(components);
        for (size_t k = 0; k < comp_count; ++k) {
            GameObjectComponent *component = list_get(components, k);
            if (component->w_type == &CollisionBodyComponentType) {
                list_add(self->collision_components, component);
                list_add(self->sweep_list_x, component);
                break;
            }
        }
    }
}

int coll_compare_left_edge(const void *a, const void *b)
{
    CollisionBody *bd_a = *(CollisionBody**)a;
    CollisionBody *bd_b = *(CollisionBody**)b;
    GameObject *parent_a = comp_get_parent(bd_a);
    GameObject *parent_b = comp_get_parent(bd_b);
    
    FixNumber diff = parent_a->position.x + bd_a->body_rect.origin.x - parent_b->position.x + bd_b->body_rect.origin.x;
    
    if (diff < fn_zero) {
        return list_sorted_ascending;
    } else if (diff > fn_zero) {
        return list_sorted_descending;
    } else {
        return list_sorted_same;
    }
}

void c_world_test_object_collisions(CollisionWorld *self)
{
    list_sort_insertsort(self->sweep_list_x, coll_compare_left_edge);
    
    size_t count = list_count(self->sweep_list_x);
    for (size_t i = 0; i < count; ++i) {
        CollisionBody *body = list_get(self->sweep_list_x, i);
        GameObject *parent = comp_get_parent(body);
        
        FixNumber right_edge = parent->position.x + body->body_rect.origin.x + body->body_rect.size.width;

        for (size_t k = i + 1; k < count; ++k) {
            CollisionBody *other_body = list_get(self->sweep_list_x, k);
            GameObject *other_parent = comp_get_parent(other_body);
            
            if (other_parent->position.x + other_body->body_rect.origin.x >= right_edge) {
                break;
            }
            
            if (self->collision_masks[body->collision_layer] & (1 << other_body->collision_layer)) {
                FixNumber top = parent->position.y + body->body_rect.origin.y;
                FixNumber bottom = parent->position.y + body->body_rect.origin.y + body->body_rect.size.height;
                FixNumber other_top = other_parent->position.y + other_body->body_rect.origin.y;
                FixNumber other_bottom = other_parent->position.y + other_body->body_rect.origin.y + other_body->body_rect.size.height;
                if (other_bottom > top && other_top < bottom) {
                    self->collision_callback(body, other_body, self->w_callback_context);
                }
            }
        }
    }
}

void c_world_fixed_update(GameObjectComponent *comp, FixNumber dt_ms)
{
#ifdef ENABLE_PROFILER
    profiler_start_segment("Collision world");
#endif
    CollisionWorld *self = (CollisionWorld *)comp;
    
#ifdef ENABLE_PROFILER
    profiler_start_segment("Object collision");
#endif
    if (self->collision_callback) {
        c_world_test_object_collisions(self);
    }
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
    
    size_t po_count = list_count(self->collision_components);
    for (size_t i = 0; i < po_count; ++i) {
#ifdef ENABLE_PROFILER
        profiler_start_segment("Object movement");
#endif
        CollisionBody *body = (CollisionBody *)list_get(self->collision_components, i);
        GameObject *object = comp_get_parent(body);
        
        Vector2D control_movement = vec(fn_mul(body->control_movement.x, dt_ms) / 1000,
                                        fn_mul(body->control_movement.y, dt_ms) / 1000);
        Vector2D velocity_movement = vec(fn_mul(body->velocity.x, dt_ms) / 1000,
                                         fn_mul(body->velocity.y, dt_ms) / 1000);
        Vector2D target_movement = vec_vec_add(velocity_movement, control_movement);
        
        object->position = vec_vec_add(target_movement, object->position);
        
#ifdef ENABLE_PROFILER
        profiler_end_segment();
#endif
    }
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
}

GameObjectComponentType CollisionWorldComponentType = {
    { { "CollisionWorld", &c_world_destroy, &c_world_describe } },
    NULL,
    NULL,
    &c_world_start,
    NULL,
    &c_world_fixed_update
};

CollisionWorld *c_world_create(void *callback_context, collision_world_callback_t *collision_callback, uint16_t collision_masks[16])
{
    CollisionWorld *self = (CollisionWorld *)comp_alloc(sizeof(CollisionWorld));
    
    self->w_type = &CollisionWorldComponentType;
    self->collision_components = list_create_with_weak_references();
    self->sweep_list_x = list_create_with_weak_references();
    self->collision_callback = collision_callback;
    self->w_callback_context = callback_context;
    for (int i = 0; i < 16; ++i) {
        self->collision_masks[i] = collision_masks[i];
    }
    
    return self;
}

void c_world_add_child(CollisionWorld *self, void *child)
{
    if (!go_get_parent(child)) {
        go_add_child(self, child);
    }
    
    CollisionBody *body = (CollisionBody*)go_get_component(child, &CollisionBodyComponentType);
    if (body && !list_contains(self->collision_components, body)) {
        list_add(self->collision_components, body);
        list_add(self->sweep_list_x, body);
    }
}

void *c_world_remove_object_from_world(void *child)
{
    CollisionBody *body = (CollisionBody*)go_get_component(child, &CollisionBodyComponentType);
    if (!body) {
        LOG_ERROR("Trying to remove object from world: does not have body");
        return NULL;
    }
    
    GameObject *parent = go_get_parent(child);
    
    if (!parent) {
        LOG_ERROR("Trying to remove object from world: does not have parent");
        return NULL;
    }
    
    CollisionWorld *world = (CollisionWorld*)go_get_component(parent, &CollisionWorldComponentType);
    
    if (!world) {
        LOG_ERROR("Trying to remove object from world: parent does not have world");
        return NULL;
    }
    
    list_drop_item(world->collision_components, body);
    list_drop_item(world->sweep_list_x, body);
    
    return child;
}
