#ifndef scene_object_h
#define scene_object_h

#include "types.h"
#include "base_object.h"
#include "array_list.h"
#include "render_context.h"
#include "number.h"

struct GameObject;
struct SceneManager;
struct go_private;
struct GameObjectComponent;
struct GameObjectComponentType;

#define GOT_CONTENTS \
    BASE_TYPE; \
    void (*added_to_parent)(struct GameObject *); \
    void (*will_be_removed_from_parent)(struct GameObject *); \
    void (*start)(struct GameObject *); \
    void (*update)(struct GameObject *, Float); \
    void (*fixed_update)(struct GameObject *, Float, Number); \
    void (*render)(struct GameObject *, RenderContext *)

typedef struct GameObjectType {
    BASE_TYPE;
    void (*added_to_parent)(struct GameObject *);
    void (*will_be_removed_from_parent)(struct GameObject *);
    void (*start)(struct GameObject *);
    void (*update)(struct GameObject *, Float);
    void (*fixed_update)(struct GameObject *, Float, FixNumber);
    void (*render)(struct GameObject *, RenderContext *);
} GameObjectType;

#define GO_CONTENTS \
    BASE_OBJECT; \
    struct go_private *go_private; \
    Vector2D position; \
    Vector2D anchor; \
    Vector2D scale; \
    Size2D size; \
    Float rotation; \
    int32_t tag; \
    bool active; \
    bool ignore_camera; \
    bool layout_children_from_top_left

typedef struct GameObject {
    GO_CONTENTS;
} GameObject;

#define GAME_OBJECT union { \
    struct { GO_CONTENTS; }; \
    GameObject go_base; \
}

#define GAME_OBJECT_TYPE union { \
    struct { GOT_CONTENTS; }; \
    GameObjectType go_type; \
}

#define game_object_type(const_name_str, destroy, describe, added_to_parent, will_be_removed_from_parent, start, update, fixed_update, render) \
{ { { const_name_str, destroy, describe } }, added_to_parent, will_be_removed_from_parent, start, update, fixed_update, render }

GameObject *go_alloc(size_t type_size);
GameObject *go_create_empty(void);

void go_initialize(GameObject *object, struct SceneManager *mngr);
void go_start(GameObject *object);
void go_update(GameObject *object, Float dt);
void go_fixed_update(GameObject *object, Float dt, FixNumber dt_ms);
void go_render(GameObject *object, RenderContext *ctx);

void go_add_child(void *obj, void *child);
void *go_remove_from_parent(void *obj);

void go_add_component(void *obj, void *comp);

ArrayList *go_get_children(void *obj);
ArrayList *go_get_components(void *obj);
GameObject *go_get_parent(void *obj);
GameObject *go_get_root_ancestor(void *obj);
GameObject *go_get_ancestor_with_tag(void *obj, int32_t tag);
struct SceneManager *go_get_scene_manager(void *obj);
struct GameObjectComponent *go_get_component(void *obj, struct GameObjectComponentType *type);

Vector2D go_position_in_ancestor(void *obj, void *ancestor);
Float go_rotation_in_ancestor(void *obj, void *ancestor);
Float go_rotation_from_root(void *obj);

void go_schedule_destroy(void *obj);

void go_set_z_order(void *obj, int32_t z_order);
int32_t go_get_z_order(void *obj);

char *go_describe(void *obj);

GameObjectType *go_type(void *object);

void go_destroy(void *obj);

#endif /* scene_object_h */
