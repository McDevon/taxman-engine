#ifndef game_scene_h
#define game_scene_h

#include <stdlib.h>
#include "types.h"
#include "array_list.h"
#include "game_object.h"
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
    GameObject *current_scene;
    GameObject *next_scene;
    ArrayList *destroy_queue;
    Image *w_transition_dither;
    void *data;
    Controls controls;
    Number transition_length;
    Number transition_step;
    SceneTransition transition;
    bool controls_enabled;
} SceneManager;

void scene_change(SceneManager *scene_manager, GameObject *next_scene, SceneTransition transition, Number time);

SceneManager *scene_manager_create(void);

#define CREATE_SCENE_MANAGER() { { { &SceneManagerType } }, NULL, NULL, NULL, NULL, NULL, empty_controls, 0, 0, st_none, true }

#endif /* game_scene_h */
