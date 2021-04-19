#include "scene_manager.h"
#include "image_storage.h"
#include "game_object.h"
#include "platform_adapter.h"
#include <string.h>

void scenemanager_destroy(void *table);
char *scenemanager_describe(void *table);

BaseType SceneManagerType = { "SceneManager", &scenemanager_destroy, &scenemanager_describe };

void scene_change(SceneManager *scene_manager, GameObject *next_scene, SceneTransition transition, Number time)
{
    if (scene_manager->next_scene) {
        return;
    }
    scene_manager->next_scene = next_scene;
    scene_manager->transition = transition;
    scene_manager->transition_step = 0;
    scene_manager->transition_length = time;
    scene_manager->w_transition_dither = get_image("dither_blue");
}

void scenemanager_destroy(void *table)
{
    
}

char *scenemanager_describe(void *table)
{
    return platform_strdup("{}");
}
