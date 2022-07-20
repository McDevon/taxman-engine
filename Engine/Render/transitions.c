#include "transitions.h"
#include "transforms.h"
#include "constants.h"
#include "utils.h"

void draw_ltr_first_half(int32_t fade_width, int32_t dither_width, Image *dither, RenderContext *ctx)
{
    const int offset_x = 0;
    const int offset_y = 0;

    const int32_t black_width = max(fade_width - dither_width, 0);
    const int32_t dither_left_edge = fade_width - dither_width;
    const int32_t right_edge = min(fade_width, ctx->w_target_buffer->size.width);
    const int32_t height = ctx->w_target_buffer->size.height;
    const int32_t width = ctx->w_target_buffer->size.width;
    const uint32_t target_channels = image_data_channel_count(ctx->w_target_buffer);

    const uint32_t dither_tx_width = dither->rect.size.width;
    const uint32_t dither_tx_height = dither->rect.size.height;
    const uint32_t dither_data_width = dither->w_image_data->size.width;
    const uint32_t dither_channels = image_channel_count(dither);
    const uint32_t dither_origin_x = dither->rect.origin.x;
    const uint32_t dither_origin_y = dither->rect.origin.y;

    ImageBuffer *target = ctx->w_target_buffer->buffer;
    ImageBuffer *dither_buffer = dither->w_image_data->buffer;
    
    for (int32_t i = 0; i < black_width; i++) {
        for (int32_t j = 0; j < height; j++) {
            int32_t index = (i + j * width) * target_channels;
            target[index] = 0;
        }
    }
    
    for (int32_t i = black_width; i < right_edge; i++) {
        int32_t grey_val = 255 - ((i - dither_left_edge) * 255 / dither_width);
        const int32_t dither_x = (i + offset_x) % dither_tx_width;
        
        for (int32_t j = 0; j < height; j++) {
            int32_t index = (i + j * width) * target_channels;
            
            const int32_t dither_y = (j + offset_y) % dither_tx_height;
            
            const uint32_t d_index = (dither_x + dither_origin_x + (dither_y + dither_origin_y) * dither_data_width) * dither_channels;

            if (target[index] && grey_val > dither_buffer[d_index]) {
                target[index] = 0;
            }
        }
    }
}

void draw_ltr_second_half(int32_t fade_width, int32_t dither_width, Image *dither, RenderContext *ctx)
{
    const int offset_x = 0;
    const int offset_y = 0;

    const int32_t height = ctx->w_target_buffer->size.height;
    const int32_t width = ctx->w_target_buffer->size.width;
    const int32_t black_width = max(fade_width - dither_width, 0);
    const int32_t dither_right_edge = min(width - black_width, width);
    const int32_t left_edge = min(width - fade_width, width);
    const uint32_t target_channels = image_data_channel_count(ctx->w_target_buffer);

    const uint32_t dither_tx_width = dither->rect.size.width;
    const uint32_t dither_tx_height = dither->rect.size.height;
    const uint32_t dither_data_width = dither->w_image_data->size.width;
    const uint32_t dither_channels = image_channel_count(dither);
    const uint32_t dither_origin_x = dither->rect.origin.x;
    const uint32_t dither_origin_y = dither->rect.origin.y;
    
    ImageBuffer *target = ctx->w_target_buffer->buffer;
    ImageBuffer *dither_buffer = dither->w_image_data->buffer;
    
    for (int32_t i = dither_right_edge; i < width; i++) {
        for (int32_t j = 0; j < height; j++) {
            int32_t index = (i + j * width) * target_channels;
            target[index] = 0;
        }
    }
        
    for (int32_t i = max(left_edge, 0); i < dither_right_edge; i++) {
        int32_t grey_val = 255 - ((dither_width - i + left_edge) * 255 / dither_width);
        const int32_t dither_x = (i + offset_x) % dither_tx_width;

        for (int32_t j = 0; j < height; j++) {
            int32_t index = (i + j * width) * target_channels;
            
            const int32_t dither_y = (j + offset_y) % dither_tx_height;
            
            const uint32_t d_index = (dither_x + dither_origin_x + (dither_y + dither_origin_y) * dither_data_width) * dither_channels;

            if (target[index] && grey_val > dither_buffer[d_index]) {
                target[index] = 0;
            }
        }
    }
}

void transition_swipe_ltr_step(SceneManager *scene_manager, RenderContext *ctx)
{
    const int32_t dither_width = 900;
    const int32_t full_width = dither_width + SCREEN_WIDTH;

    const Number half_time = scene_manager->transition_length / 2;
    if (scene_manager->transition_step < half_time) {
        draw_ltr_first_half(nb_to_int(nb_mul(nb_from_int(full_width), nb_div(scene_manager->transition_step, half_time))), dither_width, scene_manager->w_transition_dither, ctx);
    } else {
        ctx->render_transform = render_camera_get_transform(ctx->render_camera);
        go_render(scene_manager->current_scene, ctx);
        draw_ltr_second_half(nb_to_int(nb_mul(nb_from_int(full_width), nb_div((scene_manager->transition_length - scene_manager->transition_step), half_time))), dither_width, scene_manager->w_transition_dither, ctx);
    }
}

void draw_fade_black(int32_t fade, Image *dither, RenderContext *ctx)
{
    const int offset_x = 0;
    const int offset_y = 0;

    const int32_t width = ctx->w_target_buffer->size.width;
    const int32_t height = ctx->w_target_buffer->size.height;
    const uint32_t target_channels = image_data_channel_count(ctx->w_target_buffer);

    const uint32_t dither_tx_width = dither->rect.size.width;
    const uint32_t dither_tx_height = dither->rect.size.height;
    const uint32_t dither_data_width = dither->w_image_data->size.width;
    const uint32_t dither_channels = image_channel_count(dither);
    const uint32_t dither_origin_x = dither->rect.origin.x;
    const uint32_t dither_origin_y = dither->rect.origin.y;
    
    ImageBuffer *target = ctx->w_target_buffer->buffer;
    ImageBuffer *dither_buffer = dither->w_image_data->buffer;

    for (int32_t i = 0; i < width; i++) {
        for (int32_t j = 0; j < height; j++) {
            int32_t index = (i + j * width) * target_channels;
            
            const int32_t dither_x = (i + offset_x) % dither_tx_width;
            const int32_t dither_y = (j + offset_y) % dither_tx_height;
            
            const uint32_t d_index = (dither_x + dither_origin_x + (dither_y + dither_origin_y) * dither_data_width) * dither_channels;

            if (target[index] && fade <= dither_buffer[d_index]) {
                target[index] = 0;
            }
        }
    }
}

void transition_fade_black_step(SceneManager *scene_manager, RenderContext *ctx)
{
    const Number half_time = scene_manager->transition_length / 2;
    if (scene_manager->transition_step < half_time) {
        draw_fade_black(255 - nb_to_int(nb_div(nb_mul(scene_manager->transition_step, nb_from_int(255)), half_time)), scene_manager->w_transition_dither, ctx);
    } else {
        ctx->render_transform = render_camera_get_transform(ctx->render_camera);
        go_render(scene_manager->current_scene, ctx);
        draw_fade_black(255 - nb_to_int(nb_div(nb_mul((scene_manager->transition_length - scene_manager->transition_step), nb_from_int(255)), half_time)), scene_manager->w_transition_dither, ctx);
    }
}
