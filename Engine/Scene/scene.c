#include "scene.h"
#include "scene_private.h"
#include "platform_adapter.h"
#include <stdarg.h>

void scene_destroy(void *obj)
{
    Scene *scene = (Scene*)obj;
    if (scene->scene_private->sprite_sheet_names) {
        destroy(scene->scene_private->sprite_sheet_names);
        scene->scene_private->sprite_sheet_names = NULL;
    }
    if (scene->scene_private->grid_atlas_infos) {
        destroy(scene->scene_private->grid_atlas_infos);
        scene->scene_private->grid_atlas_infos = NULL;
    }
    if (scene->scene_private->audio_effects) {
        destroy(scene->scene_private->audio_effects);
        scene->scene_private->audio_effects = NULL;
    }
    platform_free(scene->scene_private);
    scene->scene_private = NULL;
    
    go_destroy(scene);
}

char *scene_describe(void *scene)
{
    return go_describe(scene);
}

Scene *scene_alloc(size_t type_size)
{
    Scene * scene = (Scene *)go_alloc(type_size);
    scene->scene_private = platform_calloc(1, sizeof(struct scene_private));

    scene->scene_private->sprite_sheet_names = list_create_with_destructor(&platform_free);
    scene->scene_private->grid_atlas_infos = list_create_with_destructor(&grid_atlas_info_destroy);
    scene->scene_private->audio_effects = list_create_with_destructor(&platform_free);

    return scene;
}

void scene_set_required_image_asset_names(void *obj, ArrayList *sprite_sheet_names)
{
    Scene *scene = (Scene*)obj;
    if (scene->scene_private->sprite_sheet_names != NULL) {
        destroy(scene->scene_private->sprite_sheet_names);
        scene->scene_private->sprite_sheet_names = NULL;
    }
    scene->scene_private->sprite_sheet_names = sprite_sheet_names;
}

void scene_set_required_grid_atlas_infos(void *obj, ArrayList *grid_atlas_infos)
{
    Scene *scene = (Scene*)obj;
    if (scene->scene_private->grid_atlas_infos != NULL) {
        destroy(scene->scene_private->grid_atlas_infos);
        scene->scene_private->grid_atlas_infos = NULL;
    }
    scene->scene_private->grid_atlas_infos = grid_atlas_infos;
}

void scene_set_required_audio_effects(void *obj, ArrayList *audio_effects)
{
    Scene *scene = (Scene*)obj;
    if (scene->scene_private->audio_effects != NULL) {
        destroy(scene->scene_private->audio_effects);
        scene->scene_private->audio_effects = NULL;
    }
    scene->scene_private->audio_effects = audio_effects;
}

ArrayList *__list_of_grid_atlas_infos(GridAtlasInfo *first, ...)
{
    ArrayList *list = list_create_with_destructor(&grid_atlas_info_destroy);
        
    va_list arg;
    va_start (arg, first);
    for (GridAtlasInfo *info = first; info != NULL; info = va_arg(arg, GridAtlasInfo *)) {
        list_add(list, info);
    }
    va_end (arg);

    return list;
}
