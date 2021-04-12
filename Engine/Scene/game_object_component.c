#include "game_object_component.h"
#include "game_object_component_private.h"
#include "game_object_private.h"
#include "engine_log.h"
#include "string_builder.h"
#include "platform_adapter.h"

GameObjectComponent *comp_alloc(size_t type_size)
{
    GameObjectComponent *object = platform_calloc(1, type_size);
    object->comp_private = platform_calloc(1, sizeof(struct go_comp_private));
    object->comp_private->w_parent = NULL;
    object->active = true;
    
    return object;
}

inline GameObjectComponentType *comp_type(GameObjectComponent *obj)
{
    return (GameObjectComponentType *)obj->w_type;
}

void comp_remove_from_parent(void *obj)
{
    GameObjectComponent *comp = (GameObjectComponent *)obj;
    if (!comp->comp_private->w_parent) {
        LOG_ERROR("Trying to remove component from parent, has no parent");
        return;
    }
    ArrayList *par_children = comp->comp_private->w_parent->go_private->children;
    size_t count = list_count(par_children);
    for (size_t i = 0; i < count; ++i) {
        if (list_get(par_children, i) == comp) {
            list_drop_index(par_children, i);
            break;
        }
    }
}

inline GameObject *comp_get_parent(void *obj)
{
    GameObjectComponent *comp = (GameObjectComponent *)obj;
    return comp->comp_private->w_parent;
}

inline struct SceneManager *comp_get_scene_manager(void *obj)
{
    GameObjectComponent *comp = (GameObjectComponent *)obj;
    GameObject *parent = comp_get_parent(comp);
    return go_get_scene_manager(parent);
}

GameObjectComponent *comp_get_component(GameObjectComponent *self, GameObjectComponentType *type)
{
    GameObject *parent = comp_get_parent(self);
    return go_get_component(parent, type);
}

char *comp_describe(void *object)
{
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "Parent object [");
    sb_append_string(sb, object_type_string(comp_get_parent(object)));
    sb_append_string(sb, "]: ");
    char *parent_description = describe(comp_get_parent(object));
    sb_append_string(sb, parent_description);
    char *description = sb_get_string(sb);
    
    destroy(sb);
    platform_free(parent_description);
    
    return description;
}

void comp_destroy(void *object)
{
    GameObjectComponent *comp = (GameObjectComponent *)object;
    platform_free(comp->comp_private);
    comp->comp_private = NULL;
}
