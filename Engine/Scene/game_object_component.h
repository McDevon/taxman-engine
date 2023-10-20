#ifndef go_component_h
#define go_component_h

#include "base_object.h"
#include "types.h"
#include "scene_manager.h"

struct go_comp_private;
struct GameObjectComponent;

typedef struct GameObjectComponentType {
    BASE_TYPE;
    void (*added_to_object)(struct GameObjectComponent *);
    void (*object_will_be_removed_from_parent)(struct GameObjectComponent *);
    void (*start)(struct GameObjectComponent *);
    void (*update)(struct GameObjectComponent *, Float);
    void (*fixed_update)(struct GameObjectComponent *, Float, FixNumber);
} GameObjectComponentType;

#define GO_COMPONENT_CONTENTS \
    BASE_OBJECT; \
    struct go_comp_private *comp_private; \
    bool active

typedef struct GameObjectComponent {
    GO_COMPONENT_CONTENTS;
} GameObjectComponent;

#define GAME_OBJECT_COMPONENT union { \
    struct { GO_COMPONENT_CONTENTS; }; \
    GameObjectComponent component_base; \
}

#define go_component_type(const_name_str, destroy, describe, added_to_object, object_will_be_removed_from_parent, start, update, fixed_update) \
{ { { const_name_str, destroy, describe } }, added_to_object, object_will_be_removed_from_parent, start, update, fixed_update }

GameObjectComponent *comp_alloc(size_t type_size);

void comp_remove_from_parent(void *obj);

GameObject *comp_get_parent(void *component);
struct SceneManager *comp_get_scene_manager(void *component);
GameObjectComponentType *comp_type(void *component);

GameObjectComponent *comp_get_component(void *component, GameObjectComponentType *type);

void comp_schedule_destroy(void *component);

void comp_destroy(void *object);
char *comp_describe(void *object);

#endif /* go_component_h */
