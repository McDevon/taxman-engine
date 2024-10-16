#ifndef game_scene_h
#define game_scene_h

#include <stdlib.h>
#include "types.h"
#include "array_list.h"
#include "hash_table.h"
#include "scene.h"
#include "base_object.h"

typedef enum {
    st_none,
    st_instant,
    st_swipe_left_to_right,
    st_fade_black
} SceneTransition;

extern BaseType SceneManagerType;

typedef struct SceneManager {
    BASE_OBJECT;
    Scene *current_scene;
    Scene *next_scene;
    ArrayList *go_destroy_queue;
    ArrayList *comp_destroy_queue;
    ArrayList *loaded_image_file_names;
    ArrayList *loaded_sprite_sheet_names;
    ArrayList *loaded_grid_atlas_names;
    ArrayList *loaded_audio_effect_names;
    HashTable *assets_in_waiting;
    context_callback_t *loading_callback;
    void *loading_callback_context;
    Image *w_transition_dither;
    void *data;
    Controls controls;
    Float transition_length;
    Float transition_step;
    SceneTransition transition;
    bool controls_enabled;
    bool running;
} SceneManager;

void scene_change(SceneManager *scene_manager, Scene *next_scene, SceneTransition transition, Float time);

SceneManager *scene_manager_create(void);

void scene_manager_load_scene_assets(SceneManager *self, Scene *next_scene, context_callback_t callback, void *context);

#define empty_scene_manager { { { &SceneManagerType } }, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, { (Float)0, (Float)0, empty_button_controls, empty_button_controls, empty_button_controls }, 0, 0, st_none, true, false }

#endif /* game_scene_h */
