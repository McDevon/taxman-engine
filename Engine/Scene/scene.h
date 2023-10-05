#ifndef scene_h
#define scene_h

#include "game_object.h"
#include "types.h"

struct scene_private;
struct GridAtlasInfo;

typedef struct SceneType {
    GAME_OBJECT_TYPE;
} SceneType;

#define SCENE_CONTENTS \
    GAME_OBJECT; \
    struct scene_private *scene_private

typedef struct Scene {
    SCENE_CONTENTS;
} Scene;

#define SCENE union { \
    struct { SCENE_CONTENTS; }; \
    Scene scene_base; \
}

#define scene_type(const_name_str, destroy, describe, added_to_parent, will_be_removed_from_parent, start, update, fixed_update, render) \
{ { { { { const_name_str, destroy, describe } }, added_to_parent, will_be_removed_from_parent, start, update, fixed_update, render } } }

/**
    Scene is a special game object, which knows which assets it needs to have loaded.
    SceneManager can then load and unload the assets automatically when switching scenes.
 */
Scene *scene_alloc(size_t type_size);

void scene_set_required_image_asset_names(void *scene, ArrayList *sprite_sheet_names);
void scene_set_required_grid_atlas_infos(void *scene, ArrayList *grid_atlas_infos);

ArrayList *__list_of_grid_atlas_infos(struct GridAtlasInfo *, ...);

#define list_of_grid_atlas_infos(...) __list_of_grid_atlas_infos(__VA_ARGS__, (struct GridAtlasInfo*)NULL)

#endif /* scene_h */
