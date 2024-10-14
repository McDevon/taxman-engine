#include "scene_manager.h"
#include "image_storage.h"
#include "game_object.h"
#include "platform_adapter.h"
#include "string_utils.h"
#include "utils.h"
#include "scene.h"
#include "scene_private.h"
#include "string_builder.h"
#include "engine_log.h"
#include "audio_player.h"
#include <string.h>

void scenemanager_destroy(void *table);
char *scenemanager_describe(void *table);

BaseType SceneManagerType = { "SceneManager", &scenemanager_destroy, &scenemanager_describe };

void scene_change(SceneManager *scene_manager, Scene *next_scene, SceneTransition transition, Float time)
{
    if (scene_manager->next_scene) {
        return;
    }
    scene_manager->next_scene = next_scene;
    scene_manager->transition = transition;
    scene_manager->transition_step = 0;
    scene_manager->transition_length = time;
    scene_manager->w_transition_dither = get_image("dither_blue.png");
}

void scene_manager_asset_loaded_callback(const char *asset_name, bool success, void *context)
{
    SceneManager *self = (SceneManager*)context;
    
    int result = hashtable_remove(self->assets_in_waiting, asset_name);
    if (result) {
        LOG_ERROR("Asset name not found: %s", asset_name);
    }
    if (hashtable_count(self->assets_in_waiting) == 0) {
        self->loading_callback(self->loading_callback_context);
        self->loading_callback_context = NULL;
        self->loading_callback = NULL;
    }
}

void scene_manager_load_scene_assets(SceneManager *self, Scene *next_scene, context_callback_t callback, void *context)
{
    self->running = false;
    self->loading_callback = callback;
    self->loading_callback_context = context;
    
    ArrayList *images = list_create_with_weak_references();
    ArrayList *sprite_sheets = list_create_with_weak_references();
    ArrayList *grid_atlas_infos = list_create_with_weak_references();
    ArrayList *audio_effects = list_create_with_weak_references();
    ArrayList *w_grid_atlas_infos = next_scene->scene_private->grid_atlas_infos;
    
    for_each_begin(char *, image_file, next_scene->scene_private->sprite_sheet_names) {
        if (str_ends_with(image_file, ".png")) {
            if (list_contains_string(self->loaded_image_file_names, image_file)) {
                continue;
            }
            list_add(images, image_file);
        } else {
            if (list_contains_string(self->loaded_sprite_sheet_names, image_file)) {
                continue;
            }
            list_add(sprite_sheets, image_file);
        }
    }
    for_each_end

    for_each_begin(char *, asset_name, images) {
        hashtable_put(self->assets_in_waiting, asset_name, NULL);
        list_add(self->loaded_image_file_names, platform_strdup(asset_name));
    }
    for_each_end;
    for_each_begin(char *, asset_name, sprite_sheets) {
        hashtable_put(self->assets_in_waiting, asset_name, NULL);
        list_add(self->loaded_sprite_sheet_names, platform_strdup(asset_name));
    }
    for_each_end;
    for_each_begin(GridAtlasInfo *, info, w_grid_atlas_infos) {
        if (list_contains_string(self->loaded_grid_atlas_names, info->file_name)) {
            continue;
        }
        list_add(grid_atlas_infos, info);
        hashtable_put(self->assets_in_waiting, info->file_name, NULL);
        list_add(self->loaded_grid_atlas_names, platform_strdup(info->file_name));
    }
    for_each_end;
    for_each_begin(char *, audio_file, next_scene->scene_private->audio_effects) {
        if (list_contains_string(self->loaded_audio_effect_names, audio_file)) {
            continue;
        }
        list_add(audio_effects, audio_file);
        hashtable_put(self->assets_in_waiting, audio_file, NULL);
        list_add(self->loaded_audio_effect_names, platform_strdup(audio_file));
    }
    for_each_end

    describe_debug_to_log(self->assets_in_waiting);
    
    for_each_begin(char *, image_name, images) {
        load_image_data(image_name, true, &scene_manager_asset_loaded_callback, self);
    }
    for_each_end;
    for_each_begin(char *, sprite_sheet, sprite_sheets) {
        load_sprite_sheet(sprite_sheet, &scene_manager_asset_loaded_callback, self);
    }
    for_each_end;
    for_each_begin(GridAtlasInfo *, info, grid_atlas_infos) {
        load_grid_atlas(info->file_name, info->tile_size, &scene_manager_asset_loaded_callback, self);
    }
    for_each_end;
    for_each_begin(char *, audio_file, audio_effects) {
        audio_load_file(audio_file, &scene_manager_asset_loaded_callback, self);
    }
    for_each_end;

    destroy(images);
    destroy(sprite_sheets);
    destroy(grid_atlas_infos);
    destroy(audio_effects);

    if (hashtable_count(self->assets_in_waiting) == 0 && self->loading_callback != NULL) {
        self->loading_callback(self->loading_callback_context);
        self->loading_callback_context = NULL;
        self->loading_callback = NULL;
    }
}

void scenemanager_destroy(void *object)
{
    SceneManager *self = (SceneManager *)object;
    
    destroy(self->go_destroy_queue);
    self->go_destroy_queue = NULL;
    
    destroy(self->comp_destroy_queue);
    self->comp_destroy_queue = NULL;

    if (self->next_scene) {
        destroy(self->next_scene);
        self->next_scene = NULL;
    }
    if (self->current_scene) {
        destroy(self->current_scene);
        self->current_scene = NULL;
    }
}

char *scenemanager_describe(void *object)
{
    return platform_strdup("{}");
}

SceneManager *scene_manager_create()
{
    SceneManager *manager = platform_calloc(1, sizeof(SceneManager));
    manager->w_type = &SceneManagerType;
    
    manager->current_scene = NULL;
    manager->next_scene = NULL;
    manager->go_destroy_queue = list_create_with_weak_references();
    manager->comp_destroy_queue = list_create_with_weak_references();
    manager->loaded_image_file_names = list_create_with_destructor(&platform_free);
    manager->loaded_sprite_sheet_names = list_create_with_destructor(&platform_free);
    manager->loaded_grid_atlas_names = list_create_with_destructor(&grid_atlas_info_destroy);
    manager->loaded_audio_effect_names = list_create_with_destructor(&platform_free);
    manager->assets_in_waiting = hashtable_create();

    manager->data = NULL;
    
    return manager;
}
