#include "game_object.h"
#include "game_object_private.h"
#include "game_object_component.h"
#include "game_object_component_private.h"
#include "engine_log.h"
#include "scene_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include "transforms.h"
#include "string_builder.h"
#include "platform_adapter.h"

static GameObjectType PlainGameObjectType = {
    { { "GameObject", &go_destroy, &go_describe } },
    NULL, NULL, NULL, NULL, NULL, NULL
};

inline GameObjectType *go_type(GameObject *object)
{
    return (GameObjectType *)object->w_type;
}

GameObject *go_alloc(size_t type_size)
{
    GameObject *object = platform_calloc(1, type_size);
    object->go_private = platform_calloc(1, sizeof(struct go_private));
    object->go_private->children = list_create();
    object->go_private->components = list_create();
    object->go_private->w_parent = NULL;
    object->go_private->z_order = 0;
    object->go_private->z_order_dirty = false;
    object->go_private->start_called = false;
    object->active = true;
    object->ignore_camera = false;
    object->scale = (Vector2D){ nb_one, nb_one };
    
    return object;
}

GameObject *go_create_empty()
{
    GameObject *go = go_alloc(sizeof(GameObject));
    go->w_type = &PlainGameObjectType;
    return go;
}

void go_destroy(void *object)
{
    GameObject *obj = (GameObject *)object;
    
    destroy(obj->go_private->children);
    destroy(obj->go_private->components);
    obj->go_private->children = NULL;
    obj->go_private->components = NULL;
    platform_free(obj->go_private);
    obj->go_private = NULL;
}

void go_initialize(GameObject *object, struct SceneManager *mngr)
{
    object->go_private->w_scene_manager = mngr;
    GameObjectType *type = go_type(object);
    if (type->added_to_parent) {
        type->added_to_parent(object);
    }
}

void go_start(GameObject *object)
{
    if (object->go_private->start_called) {
        return;
    }
    GameObjectType *type = go_type(object);
    if (type->start) {
        type->start(object);
    }
    
    ArrayList *components = object->go_private->components;
    size_t c_count = list_count(components);
    for (size_t i = 0; i < c_count; ++i) {
        GameObjectComponent *comp = list_get(components, i);
        GameObjectComponentType *c_type = comp_type(comp);
        if (c_type->start) {
            c_type->start(comp);
        }
    }
    
    ArrayList *list = object->go_private->children;
    size_t count = list_count(list);
    for (size_t i = 0; i < count; ++i) {
        go_start((GameObject *)list_get(list, i));
    }
    
    object->go_private->start_called = true;
}

void go_update(GameObject *object, Number dt_ms)
{
    if (!object->active) {
        return;
    }
    GameObjectType *type = go_type(object);
    
    if (type->update) {
        type->update(object, dt_ms);
    }
    
    ArrayList *components = object->go_private->components;
    size_t c_count = list_count(components);
    for (size_t i = 0; i < c_count; ++i) {
        GameObjectComponent *comp = list_get(components, i);
        GameObjectComponentType *c_type = comp_type(comp);

        if (comp->active && c_type->update) {
            c_type->update(comp, dt_ms);
        }
    }
    
    ArrayList *list = object->go_private->children;
    size_t count = list_count(list);
    for (size_t i = 0; i < count; ++i) {
        go_update((GameObject *)list_get(list, i), dt_ms);
    }
}

void go_fixed_update(GameObject *object, Number dt_ms)
{
    if (!object->active) {
        return;
    }

    GameObjectType *type = go_type(object);
    if (type->fixed_update) {
        type->fixed_update(object, dt_ms);
    }
    
    ArrayList *components = object->go_private->components;
    size_t c_count = list_count(components);
    for (size_t i = 0; i < c_count; ++i) {
        GameObjectComponent *comp = list_get(components, i);
        GameObjectComponentType *c_type = comp_type(comp);
        
        if (comp->active && c_type->fixed_update) {
            c_type->fixed_update(comp, dt_ms);
        }
    }
    
    ArrayList *list = object->go_private->children;
    size_t count = list_count(list);
    for (size_t i = 0; i < count; ++i) {
        go_fixed_update((GameObject *)list_get(list, i), dt_ms);
    }
}

int go_compare_z_order(const void *a, const void *b)
{
    GameObject *go_a = *(GameObject **)a;
    GameObject *go_b = *(GameObject **)b;
    
    if (go_a->go_private->z_order < go_b->go_private->z_order) {
        return list_sorted_ascending;
    } else if (go_a->go_private->z_order > go_b->go_private->z_order) {
        return list_sorted_descending;
    } else {
        return list_sorted_same;
    }
}

void go_render(GameObject *object, RenderContext *ctx)
{
    if (!object->active) {
        return;
    }

    GameObjectType *type = go_type(object);
    ArrayList *list = object->go_private->children;

    if (object->go_private->z_order_dirty) {
        list_sort(list, go_compare_z_order);
        object->go_private->z_order_dirty = false;
    }
    
    size_t count = list_count(list);
    size_t i = 0;
    
    AffineTransform transform = object->ignore_camera ? af_identity() : ctx->render_transform;
    
    AffineTransform position = af_scale(af_identity(), object->scale);
    position = af_rotate(position, object->rotation);
    position = af_translate(position, object->position);
    position = af_af_multiply(transform, position);

    for (; i < count; ++i) {
        GameObject *child = (GameObject *)list_get(list, i);
        if (child->go_private->z_order >= 0) {
            break;
        }
        ctx->render_transform = position;
        go_render(child, ctx);
    }
    
    if (type->render) {
        ctx->render_transform = transform;
        type->render(object, ctx);        
    }
    
    for (; i < count; ++i) {
        ctx->render_transform = position;
        go_render((GameObject *)list_get(list, i), ctx);
    }
}

void go_set_scene_manager_recursively(void *obj, SceneManager *scene_manager)
{
    GameObject *go = (GameObject *)obj;
    go->go_private->w_scene_manager = scene_manager;
    
    ArrayList *children = go->go_private->children;
    size_t count = list_count(children);
    for (size_t i = 0; i < count; ++i) {
        go_set_scene_manager_recursively(list_get(children, i), scene_manager);
    }
}

void go_add_child(void *obj, void *child)
{
    GameObject *go = (GameObject *)obj;
    GameObject *goc = (GameObject *)child;
    
    if (goc->go_private->w_parent) {
        LOG_ERROR("Trying to add object as child, already has parent");
        return;
    }
    
    list_add(go->go_private->children, goc);
    goc->go_private->w_parent = go;
    go_set_scene_manager_recursively(goc, go->go_private->w_scene_manager);
    
    go->go_private->z_order_dirty = true;

    GameObjectType *c_type = go_type(goc);
    if (c_type->added_to_parent) {
        c_type->added_to_parent(goc);
    }
    
    if (go->go_private->start_called && !goc->go_private->start_called) {
        go_start(goc);
    }
}

void *go_remove_from_parent(void *obj)
{
    GameObject *go = (GameObject *)obj;
    if (!go->go_private->w_parent) {
        LOG_ERROR("Trying to remove game object from parent, has no parent");
        return NULL;
    }
    ArrayList *components = go->go_private->components;
    size_t c_count = list_count(components);
    for (size_t i = 0; i < c_count; ++i) {
        GameObjectComponent *comp = list_get(components, i);
        GameObjectComponentType *c_type = comp_type(comp);
        if (c_type->object_will_be_removed_from_parent) {
            c_type->object_will_be_removed_from_parent(comp);
        }
    }
    GameObjectType *c_type = go_type(go);
    if (c_type->will_be_removed_from_parent) {
        c_type->will_be_removed_from_parent(go);
    }
    ArrayList *par_children = go->go_private->w_parent->go_private->children;
    go->go_private->w_parent = NULL;
    return list_drop_item(par_children, go);
}

void go_add_component(void *obj, void *comp)
{
    GameObject *go = (GameObject *)obj;
    GameObjectComponent *component = (GameObjectComponent *)comp;
    
    if (component->comp_private->w_parent) {
        LOG_ERROR("Trying to add component, already has parent");
        return;
    }
    
    list_add(go->go_private->components, component);
    component->comp_private->w_parent = go;
    
    GameObjectComponentType *c_type = comp_type(component);
    if (c_type->added_to_object) {
        c_type->added_to_object(component);
    }
    
    if (go->go_private->start_called && !component->comp_private->start_called) {
        if (c_type->start) {
            c_type->start(component);
        }
        component->comp_private->start_called = true;
    }
}

GameObjectComponent *go_get_component(GameObject *self, GameObjectComponentType *type)
{
    ArrayList *components = go_get_components(self);
    size_t comp_count = list_count(components);
    for (size_t i = 0; i < comp_count; ++i) {
        GameObjectComponent *comp = list_get(components, i);
        if (comp->w_type == type) {
            return comp;
        }
    }
    return NULL;
}

void go_set_z_order(void *obj, int32_t z_order)
{
    GameObject *go = (GameObject *)obj;
    go->go_private->z_order = z_order;
    
    if (go->go_private->w_parent) {
        go->go_private->w_parent->go_private->z_order_dirty = true;
    }
}

int32_t go_get_z_order(void *obj)
{
    GameObject *go = (GameObject *)obj;
    return go->go_private->z_order;
}

char *go_describe(void *go)
{
    GameObject *obj = (GameObject *)go;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "pos: ");
    sb_append_vector2d(sb, obj->position);
    sb_append_string(sb, " size: ");
    sb_append_size2d(sb, obj->size);
    sb_append_string(sb, " z: ");
    sb_append_int(sb, obj->go_private->z_order);
    sb_append_string(sb, " active: ");
    sb_append_string(sb, obj->active ? "true" : "false");

    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

ArrayList *go_get_children(void *obj)
{
    GameObject *go = (GameObject *)obj;
    return go->go_private->children;
}

ArrayList *go_get_components(void *obj)
{
    GameObject *go = (GameObject *)obj;
    return go->go_private->components;
}

GameObject *go_get_parent(void *obj)
{
    GameObject *go = (GameObject *)obj;
    return go->go_private->w_parent;
}

GameObject *go_get_root_ancestor(void *obj)
{
    GameObject *root = (GameObject *)obj;
    while (root->go_private->w_parent) {
        root = root->go_private->w_parent;
    }
    return root;
}

GameObject *go_get_ancestor_with_tag(void *obj, int32_t tag)
{
    GameObject *root = (GameObject *)obj;
    while (root) {
        if (root->tag == tag) {
            return root;
        }
        root = root->go_private->w_parent;
    }
    return root;
}

AffineTransform go_recursive_position_search(GameObject *current, GameObject *ancestor)
{
    if (current == ancestor) {
        return af_identity();
    }
    
    AffineTransform ctx = go_recursive_position_search(current->go_private->w_parent, ancestor);

    AffineTransform pos = af_identity();
    
    pos = af_scale(pos, current->scale);
    pos = af_rotate(pos, current->rotation);
    pos = af_translate(pos, current->position);
    pos = af_af_multiply(ctx, pos);
    
    return pos;
}

Number go_recursive_rotation_search(GameObject *current, GameObject *ancestor)
{
    if (current == ancestor) {
        return nb_zero;
    }
    
    Number rotation = go_recursive_rotation_search(current->go_private->w_parent, ancestor);
    
    return rotation + current->rotation;
}

Vector2D go_position_in_ancestor(void *obj, void *ancestor)
{
    GameObject *current = (GameObject *)obj;
    
    if (!current->go_private->w_parent) {
        return vec_zero();
    }
    
    AffineTransform pos = go_recursive_position_search(current, ancestor);
    
    return vec(pos.i13, pos.i23);
}

Number go_rotation_in_ancestor(void *obj, void *ancestor)
{
    GameObject *current = (GameObject *)obj;
    
    if (!current->go_private->w_parent) {
        return nb_zero;
    }
        
    return go_recursive_rotation_search(current, ancestor);
}

void go_schedule_destroy(void *obj)
{
    SceneManager *scene_manager = go_get_scene_manager(obj);
    if (!scene_manager) {
        if (go_get_parent(obj)) {
            go_remove_from_parent(obj);
        }
        destroy(obj);
    } else {
        if (!list_contains(scene_manager->destroy_queue, obj)) {
            list_add(scene_manager->destroy_queue, obj);
        }
    }
}

struct SceneManager *go_get_scene_manager(void *obj)
{
    GameObject *go = (GameObject *)obj;
    return go->go_private->w_scene_manager;
}
