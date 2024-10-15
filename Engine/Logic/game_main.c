#include "game_main.h"
#include "game_display.h"
#include "image_render.h"
#include "image_storage.h"
#include "scene_manager.h"
#include "transitions.h"
#include <stdlib.h>
#include "engine_log.h"
#include "transforms.h"
#include "platform_adapter.h"
#include "types.h"
#include "constants.h"
#include "game_object_component.h"
#include "crank_utils.h"

#include "profiler.h"
#include "profiler_internal.h"

#define file_private static

file_private ImageData _screen = { { { NULL } }, NULL, { SCREEN_WIDTH, SCREEN_HEIGHT }, 0, NULL /*image_settings_alpha | image_settings_rgb*/ };
file_private RenderContext _ctx = { { { &RenderContextType } }, NULL, NULL, NULL, NULL, NULL, NULL, NULL, { 0, 0, 0, 0, 0, 0 }, false, true };

file_private SceneManager _scene_manager = empty_scene_manager;
file_private Float _fixed_dt_counter = 0;

file_private ScreenRenderOptions _screen_options = { NULL, NULL, { SCREEN_WIDTH, SCREEN_HEIGHT }, { 0, 0 }, false };

file_private ImageBuffer *_active_screen_buffer;

RenderContext *get_main_render_context(void)
{
    return &_ctx;
}

void __start_current_scene(void *_) {
    go_initialize((GameObject *)_scene_manager.current_scene, &_scene_manager);
    go_start((GameObject *)_scene_manager.current_scene);
    _scene_manager.running = true;
}

void game_init(void *first_scene)
{
    LOG("Game init");
    ImageBuffer *screenBuffer = platform_malloc(sizeof(uint8_t) * SCREEN_WIDTH * SCREEN_HEIGHT);

    _screen.buffer = screenBuffer;
    _active_screen_buffer = screenBuffer;
    
    _ctx.w_target_buffer = &_screen;
    _ctx.render_camera = render_camera_create((Size2DInt){ SCREEN_WIDTH, SCREEN_HEIGHT });
    _ctx.rendered_rects = list_create();
    _ctx.rect_pool = list_create();
    _ctx.active_rects = list_create();
    _ctx.merge_rects = list_create_with_weak_references();
    _ctx.end_rects = list_create_with_weak_references();

    _scene_manager.go_destroy_queue = list_create_with_weak_references();
    _scene_manager.comp_destroy_queue = list_create_with_weak_references();
    
    _scene_manager.loaded_image_file_names = list_create_with_destructor(&platform_free);
    _scene_manager.loaded_sprite_sheet_names = list_create_with_destructor(&platform_free);
    _scene_manager.loaded_grid_atlas_names = list_create_with_destructor(&grid_atlas_info_destroy);
    _scene_manager.loaded_audio_effect_names = list_create_with_destructor(&platform_free);
    _scene_manager.assets_in_waiting = hashtable_create();
    
    _scene_manager.current_scene = first_scene;
    scene_manager_load_scene_assets(&_scene_manager, _scene_manager.current_scene, &__start_current_scene, NULL);
}

void switch_scene(void)
{
    destroy(_scene_manager.current_scene);
    list_clear(_scene_manager.go_destroy_queue);
    list_clear(_scene_manager.comp_destroy_queue);
    _scene_manager.current_scene = _scene_manager.next_scene;
    _scene_manager.next_scene = NULL;
    render_camera_reset(_ctx.render_camera);
    scene_manager_load_scene_assets(&_scene_manager, _scene_manager.current_scene, &__start_current_scene, NULL);
}

void transition_finish(void)
{
    _scene_manager.transition = st_none;
    _scene_manager.transition_length = 0;
    _scene_manager.transition_step = 0;
    _scene_manager.w_transition_dither = NULL;
}

void transition_step(Float delta_time_seconds)
{
    _scene_manager.transition_step += delta_time_seconds;
    
    bool transition_middle_point = false;
    
    if (_scene_manager.transition_step > _scene_manager.transition_length / 2 && _scene_manager.next_scene) {
        switch_scene();
        transition_middle_point = true;
    }
    
    switch (_scene_manager.transition) {
        case st_swipe_left_to_right:
        {
            transition_swipe_ltr_step(&_scene_manager, &_ctx, transition_middle_point);
            break;
        }
        case st_fade_black:
        {
            transition_fade_black_step(&_scene_manager, &_ctx, transition_middle_point);
            break;
        }
            
        default:
            break;
    }

    if (_scene_manager.transition_step >= _scene_manager.transition_length) {
        transition_finish();
    }
}

void scene_cleanup(void)
{
    size_t count = list_count(_scene_manager.comp_destroy_queue);
    
    if (count > 0) {
        for (size_t i = 0; i < count; ++i) {
            GameObjectComponent *comp = list_get(_scene_manager.comp_destroy_queue, i);
            comp_remove_from_parent(comp);
            destroy(comp);
        }
        
        list_clear(_scene_manager.comp_destroy_queue);
    }

    count = list_count(_scene_manager.go_destroy_queue);
    
    if (count > 0) {
        for (size_t i = 0; i < count; ++i) {
            GameObject *obj = list_get(_scene_manager.go_destroy_queue, i);
            go_remove_from_parent(obj);
            destroy(obj);
        }
        
        list_clear(_scene_manager.go_destroy_queue);
    }
}

void game_set_control_changes(Controls previous_controls)
{
    FloatPair crank = to_same_half_circle_degrees(_scene_manager.controls.crank, previous_controls.crank);
    _scene_manager.controls.crank_change = crank.a - crank.b;
    
    _scene_manager.controls.pressed.button_a = _scene_manager.controls.buttons.button_a && !previous_controls.buttons.button_a;
    _scene_manager.controls.pressed.button_b = _scene_manager.controls.buttons.button_b && !previous_controls.buttons.button_b;
    _scene_manager.controls.pressed.button_up = _scene_manager.controls.buttons.button_up && !previous_controls.buttons.button_up;
    _scene_manager.controls.pressed.button_down = _scene_manager.controls.buttons.button_down && !previous_controls.buttons.button_down;
    _scene_manager.controls.pressed.button_left = _scene_manager.controls.buttons.button_left && !previous_controls.buttons.button_left;
    _scene_manager.controls.pressed.button_right = _scene_manager.controls.buttons.button_right && !previous_controls.buttons.button_right;
    _scene_manager.controls.pressed.button_menu = _scene_manager.controls.buttons.button_menu && !previous_controls.buttons.button_menu;
    
    _scene_manager.controls.released.button_a = !_scene_manager.controls.buttons.button_a && previous_controls.buttons.button_a;
    _scene_manager.controls.released.button_b = !_scene_manager.controls.buttons.button_b && previous_controls.buttons.button_b;
    _scene_manager.controls.released.button_up = !_scene_manager.controls.buttons.button_up && previous_controls.buttons.button_up;
    _scene_manager.controls.released.button_down = !_scene_manager.controls.buttons.button_down && previous_controls.buttons.button_down;
    _scene_manager.controls.released.button_left = !_scene_manager.controls.buttons.button_left && previous_controls.buttons.button_left;
    _scene_manager.controls.released.button_right = !_scene_manager.controls.buttons.button_right && previous_controls.buttons.button_right;
    _scene_manager.controls.released.button_menu = !_scene_manager.controls.buttons.button_menu && previous_controls.buttons.button_menu;
    
}

void game_step(Float delta_time_seconds, Float crank, ButtonControls buttons)
{
    if (!_scene_manager.running) {
        return;
    }
#ifdef ENABLE_PROFILER
    switch (profiler_schedule()) {
        case prof_start:
        {
            profiler_init();
            break;
        }
        case prof_end:
        {
            char *data = profiler_get_data();
            platform_print(data);
            platform_free(data);
            
            profiler_finish();
            break;
        }
        case prof_toggle:
        {
            profiler_toggle();
            break;
        }

        default:
            break;
    }
    
    profiler_start_segment("Game loop");
#endif
    
    Controls previous_controls = _scene_manager.controls;
    
    if (_scene_manager.controls_enabled) {
        _scene_manager.controls.buttons = buttons;
        _scene_manager.controls.crank = crank;
    } else {
        _scene_manager.controls = empty_controls;
    }
    game_set_control_changes(previous_controls);
    
    if (_scene_manager.transition != st_none) {
#ifdef ENABLE_PROFILER
        profiler_start_segment("Scene transition");
#endif
        if (_scene_manager.transition == st_instant) {
            switch_scene();
            transition_finish();
        } else {
            transition_step(delta_time_seconds);
            update_buffer(_screen.buffer, &_screen_options);
        }
#ifdef ENABLE_PROFILER
        profiler_end_segment();
        profiler_end_segment();
#endif
        return;
    }
    
#ifdef ENABLE_PROFILER
    profiler_start_segment("Scene start");
#endif
    go_start((GameObject *)_scene_manager.current_scene);
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
    
#ifdef ENABLE_PROFILER
    profiler_start_segment("Fixed update");
#endif
    const Float fixed_dt = 0.0333333333f;        
    _fixed_dt_counter += delta_time_seconds;
    
    if (_fixed_dt_counter < fixed_dt) {
        _fixed_dt_counter = fixed_dt;
    }
    
    int i;
    const int max_loops = 3;
    for (i = 0; _fixed_dt_counter >= fixed_dt && i < 3; i++) {
        go_fixed_update((GameObject *)_scene_manager.current_scene, fixed_dt);
        _fixed_dt_counter -= fixed_dt;
        _scene_manager.controls.crank_change = 0.f;
        _scene_manager.controls.pressed = empty_button_controls;
        _scene_manager.controls.released = empty_button_controls;
    }
    game_set_control_changes(previous_controls);
    
    if (i >= max_loops) {
        _fixed_dt_counter = 0;
    }
    
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
    
#ifdef ENABLE_PROFILER
    profiler_start_segment("Update");
#endif
    go_update((GameObject *)_scene_manager.current_scene, delta_time_seconds);
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif

#ifdef ENABLE_PROFILER
    profiler_start_segment("Cleanup");
#endif
    scene_cleanup();
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
    
#ifdef ENABLE_PROFILER
    profiler_start_segment("Render");
#endif
    _ctx.render_transform = render_camera_get_transform(_ctx.render_camera);
    go_render((GameObject *)_scene_manager.current_scene, &_ctx);
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
    
#ifdef ENABLE_PROFILER
    profiler_start_segment("Draw screen");
#endif
    update_buffer(_active_screen_buffer, &_screen_options);
#ifdef ENABLE_PROFILER
    profiler_end_segment();
    profiler_end_segment();
#endif
}

void set_screen_dither(ImageData * screen_dither)
{
    if (image_data_has_alpha(screen_dither)) {
        LOG_WARNING("Screen dither data has alpha, which can have unwanted results");
    }
    _screen_options.screen_dither = screen_dither;
}

void set_screen_invert(bool invert)
{
    _screen_options.invert = invert;
}

void reset_screen_options(void)
{
    _screen_options.screen_dither = NULL;
    set_screen_source_buffer(&_screen);
    _screen_options.invert = false;
}

void set_screen_source_buffer(ImageData *screen_buffer)
{
    ImageData *image_data = screen_buffer ? screen_buffer : &_screen;
    _screen_options.source_size = image_data->size;
    _screen_options.source_offset = (Vector2DInt){ 0, 0 };
    _active_screen_buffer = image_data->buffer;
}

void set_screen_source_offset(Vector2DInt source_offset)
{
    _screen_options.source_offset = source_offset;
}

void set_custom_screen_update(update_buffer_t *custom_update_function)
{
    _screen_options.custom_screen_update = custom_update_function;
}
