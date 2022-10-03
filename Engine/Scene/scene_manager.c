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
    manager->data = NULL;
    
    return manager;
}
