#ifndef go_component_h
#define go_component_h

#include "base_object.h"
#include "types.h"
#include "scene_manager.h"

struct go_comp_private;
struct GameObjectComponent;

typedef struct GameObjectComponentType {
    BASE_TYPE;
    void (*added_to_parent)(struct GameObjectComponent *);
    void (*start)(struct GameObjectComponent *);
    void (*update)(struct GameObjectComponent *, Number);
    void (*fixed_update)(struct GameObjectComponent *, Number);
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

GameObjectComponent *comp_alloc(size_t type_size);

void comp_remove_from_parent(void *obj);

GameObject *comp_get_parent(void *obj);
struct SceneManager *comp_get_scene_manager(void *obj);
GameObjectComponentType *comp_type(GameObjectComponent *obj);

GameObjectComponent *comp_get_component(GameObjectComponent *obj, GameObjectComponentType *type);

void comp_destroy(void *object);
char *comp_describe(void *object);

#endif /* go_component_h */
