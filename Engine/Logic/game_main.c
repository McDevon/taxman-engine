#include "game_main.h"
#include "game_display.h"
#include "image_render.h"
#include "image_storage.h"
#include "scene_manager.h"
#include "transitions.h"
#include "test_scene.h"
#include <stdlib.h>
#include "engine_log.h"
#include "transforms.h"
#include "platform_adapter.h"
#include "types.h"

#include "profiler.h"

static ImageData _screen = { { { NULL } }, NULL, { SCREEN_WIDTH, SCREEN_HEIGHT }, 0 /*image_settings_alpha | image_settings_rgb*/ };
static RenderContext _ctx = { { { &RenderContextType } }, NULL, NULL, { 0, 0, 0, 0, 0, 0 } };

static SceneManager _scene_manager = CREATE_SCENE_MANAGER();
static Number _fixed_dt_counter = 0;

void game_init(void *first_scene)
{
    LOG("Game init");
    ImageBuffer *screenBuffer = platform_malloc(sizeof(uint8_t) * SCREEN_WIDTH * SCREEN_HEIGHT);

    _screen.buffer = screenBuffer;
    
    _ctx.target_buffer = &_screen;
    
    _scene_manager.current_scene = first_scene;
    go_initialize(_scene_manager.current_scene, &_scene_manager);    
}

void switch_scene()
{
    destroy(_scene_manager.current_scene);
    _scene_manager.current_scene = _scene_manager.next_scene;
    _scene_manager.next_scene = NULL;
    go_initialize(_scene_manager.current_scene, &_scene_manager);
    go_start(_scene_manager.current_scene);
}

void transition_finish()
{
    _scene_manager.transition = st_none;
    _scene_manager.transition_length = 0;
    _scene_manager.transition_step = 0;
    _scene_manager.transition_dither = NULL;
}

void transition_step(Number delta_time_millis)
{
    if (_scene_manager.transition_step > _scene_manager.transition_length / 2 && _scene_manager.next_scene) {
        switch_scene();
    }
    
    _scene_manager.transition_step += delta_time_millis;
    
    switch (_scene_manager.transition) {
        case st_swipe_left_to_right:
        {
            transition_swipe_ltr_step(&_scene_manager, &_ctx);
            break;
        }
        case st_fade_black:
        {
            transition_fade_black_step(&_scene_manager, &_ctx);
            break;
        }
            
        default:
            break;
    }
    
    if (_scene_manager.transition_step >= _scene_manager.transition_length) {
        transition_finish();
    }
}

void game_step(Number delta_time_millis, Controls controls)
{
#ifdef ENABLE_PROFILER
    if (controls.button_menu && !_scene_manager.controls.button_menu) {
        profiler_toggle();
    }
#endif
    _scene_manager.controls = controls;
    
    if (_scene_manager.transition != st_none) {
#ifdef ENABLE_PROFILER
        profiler_start_segment("Scene transition");
#endif
        if (_scene_manager.transition == st_instant) {
            switch_scene();
            transition_finish();
        } else {
            transition_step(delta_time_millis);
            update_buffer(_screen.buffer);
        }
#ifdef ENABLE_PROFILER
        profiler_end_segment();
#endif
        return;
    }
    
#ifdef ENABLE_PROFILER
    profiler_start_segment("Scene start");
#endif
    go_start(_scene_manager.current_scene);
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
    
#ifdef ENABLE_PROFILER
    profiler_start_segment("Fixed update");
#endif
    const Number fixed_dt = nb_from_double(33.333333333333);
        
    _fixed_dt_counter += delta_time_millis;
    
    if (_fixed_dt_counter < fixed_dt) {
        _fixed_dt_counter = fixed_dt;
    }
    
    int i;
    const int max_loops = 3;
    for (i = 0; _fixed_dt_counter >= fixed_dt && i < 3; i++) {
        go_fixed_update(_scene_manager.current_scene, fixed_dt);
        _fixed_dt_counter -= fixed_dt;
    }
    
    if (i >= max_loops) {
        _fixed_dt_counter = 0;
    }
    
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
    
#ifdef ENABLE_PROFILER
    profiler_start_segment("Update");
#endif
    go_update(_scene_manager.current_scene, delta_time_millis);
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif

#ifdef ENABLE_PROFILER
    profiler_start_segment("Render");
#endif
    _ctx.camera_matrix = af_identity();
    go_render(_scene_manager.current_scene, &_ctx);
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
    
#ifdef ENABLE_PROFILER
    profiler_start_segment("Draw screen");
#endif
    update_buffer(_screen.buffer);
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
}
