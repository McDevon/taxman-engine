#include "image_render.h"
#include <stdlib.h>
#include "number.h"
#include "transforms.h"
#include "engine_log.h"
#include "utils.h"
#include "constants.h"
#include "profiler.h"
#include <math.h>

void context_render_rect_image(RenderContext *context, const Image *image, const Vector2DInt position, const RenderOptions render_options)
{
    if (!context || !image) { return; }
    
#ifdef ENABLE_PROFILER
    profiler_start_segment("Prepare context_render_rect_image");
#endif

    const int32_t source_width = image->rect.size.width;
    const int32_t source_height = image->rect.size.height;
    const int32_t target_width = context->w_target_buffer->size.width;
    const int32_t target_height = context->w_target_buffer->size.height;
    
    const bool flip_x = render_options.flip_x;
    const bool flip_y = render_options.flip_y;
    
    const Vector2DInt draw_offset = (Vector2DInt){
        flip_x ? image->original.width - (image->offset.x + source_width) : image->offset.x,
        flip_y ? image->original.height - (image->offset.y + source_height) : image->offset.y
    };
    
    if (position.x + draw_offset.x > target_width
        || position.x + draw_offset.x + source_width < 0
        || position.y + draw_offset.y > target_height
        || position.y + draw_offset.y + source_height < 0) {
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
        return;
    }
    
    const int32_t source_data_width = image->w_image_data->size.width;
    const ImageBuffer *image_buffer = image->w_image_data->buffer;

    const int32_t target_channels = image_data_channel_count(context->w_target_buffer);
    const int32_t source_channels = image_channel_count(image);
    const int32_t source_origin_x = image->rect.origin.x;
    const int32_t source_origin_y = image->rect.origin.y;
    ImageBuffer *target = context->w_target_buffer->buffer;
    
    const int32_t start_x = max(0, -position.x - draw_offset.x);
    const int32_t end_x = min(source_width, target_width - position.x - draw_offset.x);
    const int32_t start_y = max(0, -position.y - draw_offset.y);
    const int32_t end_y = min(source_height, target_height - position.y - draw_offset.y);
    
    const bool source_has_alpha = image_has_alpha(image);
    const bool invert = render_options.invert;
    const bool stamp = render_options.stamp;
    
#ifdef ENABLE_PROFILER
    profiler_end_segment();
    profiler_start_segment("Fill context_render_rect_image");
#endif
    
    if (source_has_alpha) {
        const int32_t source_alpha_offset = image_alpha_offset(image);
        if (stamp) {
            const uint8_t stamp_color = render_options.stamp_color;
            for (int32_t j = start_y; j < end_y; j++) {
                const int32_t ctx_y = j + position.y + draw_offset.y;
                const int32_t y = flip_y * (source_height - j - 1) + !flip_y * j;
                const int32_t y_i_index = (y + source_origin_y) * source_data_width;
                const int32_t y_t_index = ctx_y * target_width;

                for (int32_t i = start_x; i < end_x; i++) {
                    const int32_t ctx_x = i + position.x + draw_offset.x;
                    const int32_t x = flip_x * (source_width - i - 1) + !flip_x * i;
                    
                    int32_t i_index = (x + source_origin_x + y_i_index) * source_channels;
                    if (image_buffer[i_index + source_alpha_offset] < 128) {
                        continue;
                    }
                    
                    int32_t t_index = (ctx_x + y_t_index) * target_channels;
                    
                    target[t_index] = stamp_color;
                }
            }
        } else {
            for (int32_t j = start_y; j < end_y; j++) {
                const int32_t ctx_y = j + position.y + draw_offset.y;
                const int32_t y = flip_y * (source_height - j - 1) + !flip_y * j;
                const int32_t y_i_index = (y + source_origin_y) * source_data_width;
                const int32_t y_t_index = ctx_y * target_width;

                for (int32_t i = start_x; i < end_x; i++) {
                    const int32_t ctx_x = i + position.x + draw_offset.x;
                    const int32_t x = flip_x * (source_width - i - 1) + !flip_x * i;
                    
                    int32_t i_index = (x + source_origin_x + y_i_index) * source_channels;
                    if (image_buffer[i_index + source_alpha_offset] < 128) {
                        continue;
                    }
                    
                    int32_t t_index = (ctx_x + y_t_index) * target_channels;
                    
                    target[t_index] = !invert * image_buffer[i_index] + invert * (255 - image_buffer[i_index]);
                }
            }
        }
    } else {
        if (stamp) {
            const uint8_t stamp_color = render_options.stamp_color;
            for (int32_t j = start_y; j < end_y; j++) {
                const int32_t ctx_y = j + position.y + draw_offset.y;
                const int32_t y_t_index = ctx_y * target_width;
                
                for (int32_t i = start_x; i < end_x; i++) {
                    const int32_t ctx_x = i + position.x + draw_offset.x;
                    
                    int32_t t_index = (ctx_x + y_t_index) * target_channels;
                    
                    target[t_index] = stamp_color;
                }
            }
        } else {
            for (int32_t j = start_y; j < end_y; j++) {
                const int32_t ctx_y = j + position.y;
                const int32_t y = flip_y * (source_height - j - 1) + !flip_y * j;
                const int32_t y_i_index = (y + source_origin_y) * source_data_width;
                const int32_t y_t_index = ctx_y * target_width;
                
                for (int32_t i = start_x; i < end_x; i++) {
                    const int32_t ctx_x = i + position.x;
                    const int32_t x = flip_x * (source_width - i - 1) + !flip_x * i;
                    
                    int32_t i_index = (x + source_origin_x + y_i_index) * source_channels;
                    int32_t t_index = (ctx_x + y_t_index) * target_channels;
                    
                    target[t_index] = !invert * image_buffer[i_index] + invert * (255 - image_buffer[i_index]);
                }
            }
        }
    }
    
    context_rect_rendered(context, start_x + position.x + draw_offset.x, end_x + position.x + draw_offset.x - 1, start_y + position.y + draw_offset.y, end_y + position.y + draw_offset.y - 1);

#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
}

void context_render_scale_image(RenderContext *context, const Image *image, const Vector2DInt position, const Vector2D scale, const RenderOptions render_options)
{
    if (!context || !image) { return; }
    
#ifdef ENABLE_PROFILER
    profiler_start_segment("Prepare context_render_rect_image");
#endif

    const int32_t source_width = image->rect.size.width;
    const int32_t source_height = image->rect.size.height;
    const int32_t target_width = context->w_target_buffer->size.width;
    const int32_t target_height = context->w_target_buffer->size.height;
    const int32_t source_scaled_width = (int32_t)nb_floor(image->rect.size.width * scale.x);
    const int32_t source_scaled_height = (int32_t)nb_floor(image->rect.size.height * scale.y);

    const bool flip_x = render_options.flip_x;
    const bool flip_y = render_options.flip_y;
    
    const Vector2DInt draw_offset = (Vector2DInt){
        flip_x ? image->original.width - (image->offset.x + source_scaled_width) : image->offset.x,
        flip_y ? image->original.height - (image->offset.y + source_scaled_height) : image->offset.y
    };
    
    if (position.x + draw_offset.x > target_width
        || position.x + draw_offset.x + source_scaled_width < 0
        || position.y + draw_offset.y > target_height
        || position.y + draw_offset.y + source_scaled_height < 0) {
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
        return;
    }
    
    const int32_t source_data_width = image->w_image_data->size.width;
    const ImageBuffer *image_buffer = image->w_image_data->buffer;

    const int32_t target_channels = image_data_channel_count(context->w_target_buffer);
    const int32_t source_channels = image_channel_count(image);
    const int32_t source_origin_x = image->rect.origin.x;
    const int32_t source_origin_y = image->rect.origin.y;
    ImageBuffer *target = context->w_target_buffer->buffer;
    
    const int32_t start_x = max(0, -position.x - draw_offset.x);
    const int32_t end_x = min(source_scaled_width, target_width - position.x - draw_offset.x);
    const int32_t start_y = max(0, -position.y - draw_offset.y);
    const int32_t end_y = min(source_scaled_height, target_height - position.y - draw_offset.y);
    
    const Float move_x = (Float)source_width / (Float)source_scaled_width;
    const Float move_y = (Float)source_height / (Float)source_scaled_height;

    const bool source_has_alpha = image_has_alpha(image);
    const bool invert = render_options.invert;
    const bool stamp = render_options.stamp;
    
#ifdef ENABLE_PROFILER
    profiler_end_segment();
    profiler_start_segment("Fill context_render_rect_image");
#endif
    
    if (source_has_alpha) {
        const int32_t source_alpha_offset = image_alpha_offset(image);
        if (stamp) {
            const uint8_t stamp_color = render_options.stamp_color;
            Float position_y = 0;
            for (int32_t j = start_y; j < end_y; j++) {
                Float position_x = 0;
                const int32_t ctx_y = j + position.y + draw_offset.y;
                const int32_t p_j = (int32_t)floorf(position_y);
                const int32_t y = flip_y * (source_height - p_j - 1) + !flip_y * p_j;
                const int32_t y_i_index = (y + source_origin_y) * source_data_width;
                const int32_t y_t_index = ctx_y * target_width;
                position_y += move_y;

                for (int32_t i = start_x; i < end_x; i++) {
                    const int32_t p_i = (int32_t)floorf(position_x);
                    position_x += move_x;
                    const int32_t ctx_x = i + position.x + draw_offset.x;
                    const int32_t x = flip_x * (source_scaled_width - p_i - 1) + !flip_x * p_i;
                    
                    int32_t i_index = (x + source_origin_x + y_i_index) * source_channels;
                    if (image_buffer[i_index + source_alpha_offset] < 128) {
                        continue;
                    }
                    
                    int32_t t_index = (ctx_x + y_t_index) * target_channels;
                    
                    target[t_index] = stamp_color;
                }
            }
        } else {
            Float position_y = 0;
            for (int32_t j = start_y; j < end_y; j++) {
                Float position_x = 0;
                const int32_t ctx_y = j + position.y + draw_offset.y;
                const int32_t p_j = (int32_t)floorf(position_y);
                const int32_t y = flip_y * (source_height - p_j - 1) + !flip_y * p_j;
                const int32_t y_i_index = (y + source_origin_y) * source_data_width;
                const int32_t y_t_index = ctx_y * target_width;
                position_y += move_y;

                for (int32_t i = start_x; i < end_x; i++) {
                    const int32_t p_i = (int32_t)floorf(position_x);
                    position_x += move_x;
                    const int32_t ctx_x = i + position.x + draw_offset.x;
                    const int32_t x = flip_x * (source_scaled_width - p_i - 1) + !flip_x * p_i;
                    
                    int32_t i_index = (x + source_origin_x + y_i_index) * source_channels;
                    if (image_buffer[i_index + source_alpha_offset] < 128) {
                        continue;
                    }
                    
                    int32_t t_index = (ctx_x + y_t_index) * target_channels;
                    
                    target[t_index] = !invert * image_buffer[i_index] + invert * (255 - image_buffer[i_index]);
                }
            }
        }
    } else {
        if (stamp) {
            const uint8_t stamp_color = render_options.stamp_color;
            for (int32_t j = start_y; j < end_y; j++) {
                const int32_t ctx_y = j + position.y + draw_offset.y;
                const int32_t y_t_index = ctx_y * target_width;
                
                for (int32_t i = start_x; i < end_x; i++) {
                    const int32_t ctx_x = i + position.x + draw_offset.x;
                    
                    int32_t t_index = (ctx_x + y_t_index) * target_channels;
                    
                    target[t_index] = stamp_color;
                }
            }
        } else {
            Float position_y = 0;
            for (int32_t j = start_y; j < end_y; j++) {
                Float position_x = 0;
                const int32_t ctx_y = j + position.y;
                const int32_t p_j = (int32_t)floorf(position_y);
                const int32_t y = flip_y * (source_height - p_j - 1) + !flip_y * p_j;
                const int32_t y_i_index = (y + source_origin_y) * source_data_width;
                const int32_t y_t_index = ctx_y * target_width;
                position_y += move_y;

                for (int32_t i = start_x; i < end_x; i++) {
                    const int32_t p_i = (int32_t)floorf(position_x);
                    position_x += move_x;
                    const int32_t ctx_x = i + position.x;
                    const int32_t x = flip_x * (source_scaled_width - p_i - 1) + !flip_x * p_i;

                    int32_t i_index = (x + source_origin_x + y_i_index) * source_channels;
                    int32_t t_index = (ctx_x + y_t_index) * target_channels;
                    
                    target[t_index] = !invert * image_buffer[i_index] + invert * (255 - image_buffer[i_index]);
                }
            }
        }
    }
    
    context_rect_rendered(context, start_x + position.x + draw_offset.x, end_x + position.x + draw_offset.x - 1, start_y + position.y + draw_offset.y, end_y + position.y + draw_offset.y - 1);

#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
}

void context_fill(RenderContext *context, uint8_t color)
{
    if (!context) { return; }

    const int32_t target_width = context->w_target_buffer->size.width;
    const int32_t target_height = context->w_target_buffer->size.height;
    const int32_t target_channels = image_data_channel_count(context->w_target_buffer);
    ImageBuffer *target = context->w_target_buffer->buffer;

    for (int32_t i = 0; i < target_width; i++) {
        for (int32_t j = 0; j < target_height; j++) {
            int32_t t_index = (i + j * target_width) * target_channels;
            target[t_index] = color;
        }
    }
}

void context_clear_white(RenderContext *context)
{
    context_fill(context, 0xff);
}

void context_clear_black(RenderContext *context)
{
    context_fill(context, 0x00);
}

void context_render(RenderContext *context, const Image *image, const RenderOptions render_options)
{
    if (!context || !image) { return; }
    
#ifdef ENABLE_PROFILER
    profiler_start_segment("Prepare context_render");
#endif
    
    const int32_t source_width = image->rect.size.width;
    const int32_t source_height = image->rect.size.height;
    const int32_t target_width = context->w_target_buffer->size.width;
    const int32_t target_height = context->w_target_buffer->size.height;
    
    const bool flip_x = render_options.flip_x;
    const bool flip_y = render_options.flip_y;
    
    const Vector2DInt draw_offset_int = (Vector2DInt){
        flip_x ? image->original.width - (image->offset.x + image->rect.size.width) : image->offset.x,
        flip_y ? image->original.height - (image->offset.y + image->rect.size.height) : image->offset.y
    };
    const Vector2D draw_offset = (Vector2D){
        nb_from_int(draw_offset_int.x),
        nb_from_int(draw_offset_int.y)
    };

    Vector2D left_up = (Vector2D){ draw_offset.x, draw_offset.y };
    Vector2D right_up = (Vector2D){ draw_offset.x + nb_from_int(source_width), draw_offset.y };
    Vector2D left_down = (Vector2D){ draw_offset.x, draw_offset.y + nb_from_int(source_height) };
    Vector2D right_down = (Vector2D){ draw_offset.x + nb_from_int(source_width), draw_offset.y + nb_from_int(source_height) };
    
    Vector2D corners[] = { left_up, right_up, left_down, right_down };
    
    Number top = nb_max_value;
    Number left = nb_max_value;
    Number bottom = nb_min_value;
    Number right = nb_min_value;
    
    for (int i = 0; i < 4; ++i) {
        Vector2D corner = corners[i];
        AffineTransform transform = af_identity();
        transform = af_translate(transform, corner);
        
        transform = af_af_multiply(context->render_transform, transform);
        
        if (transform.i13 < left) {
            left = transform.i13;
        }
        if (transform.i13 > right) {
            right = transform.i13;
        }
        if (transform.i23 < top) {
            top = transform.i23;
        }
        if (transform.i23 > bottom) {
            bottom = transform.i23;
        }
    }
    
    if (right < nb_zero || left > nb_from_int(SCREEN_WIDTH) || bottom < nb_zero || top > nb_from_int(SCREEN_HEIGHT)) {
#ifdef ENABLE_PROFILER
        profiler_end_segment();
#endif
        return;
    }
    
    const int32_t source_data_width = image->w_image_data->size.width;
    const ImageBuffer *image_buffer = image->w_image_data->buffer;
    const int32_t target_channels = image_data_channel_count(context->w_target_buffer);
    const int32_t source_channels = image_channel_count(image);
    ImageBuffer *target = context->w_target_buffer->buffer;
    
    const bool source_has_alpha = image_has_alpha(image);
    
    AffineTransformFloat inverse_camera = faf_inverse(af_to_faf(context->render_transform));
        
    int32_t i_right = min(nb_to_int(nb_ceil(right)), target_width);
    int32_t i_bottom = min(nb_to_int(nb_ceil(bottom)), target_height);
    int32_t i_left = max(nb_to_int(nb_round(left)), 0);
    int32_t i_top = max(nb_to_int(nb_round(top)), 0);
    const int32_t source_origin_x = image->rect.origin.x;
    const int32_t source_origin_y = image->rect.origin.y;

    const bool invert = render_options.invert;
    const bool stamp = render_options.stamp;

#ifdef ENABLE_PROFILER
    profiler_end_segment();
    profiler_start_segment("Fill context_render");
#endif

    if (source_has_alpha) {
        const int32_t source_alpha_offset = image_alpha_offset(image);
        const uint8_t stamp_color = render_options.stamp_color;
        if (stamp) {
            for (int32_t j = i_top; j < i_bottom; j++) {
                const int32_t y_t_index = j * target_width;

                for (int32_t i = i_left; i < i_right; i++) {
                    AffineTransformFloat t = faf_faf_multiply(inverse_camera, faf_translate(faf_identity(), (Vector2DFloat){ (Float)i, (Float)j }));
                    
                    const int32_t x = flip_x ? source_width - (int)t.i13 + draw_offset_int.x - 1: (int)t.i13 - draw_offset_int.x;
                    const int32_t y = flip_y ? source_height - (int)t.i23 + draw_offset_int.y - 1 : (int)t.i23 - draw_offset_int.y;
                    
                    if (x < 0 || x >= source_width || y < 0 || y >= source_height) {
                        continue;
                    }
                    int32_t i_index = (x + source_origin_x + (y + source_origin_y) * source_data_width) * source_channels;
                    if (image_buffer[i_index + source_alpha_offset] < 128) {
                        continue;
                    }
                    
                    int32_t t_index = (i + y_t_index) * target_channels;
                    target[t_index] = stamp_color;
                }
            }
        } else {
            for (int32_t j = i_top; j < i_bottom; j++) {
                const int32_t y_t_index = j * target_width;

                for (int32_t i = i_left; i < i_right; i++) {
                    AffineTransformFloat t = faf_faf_multiply(inverse_camera, faf_translate(faf_identity(), (Vector2DFloat){ (Float)i, (Float)j }));
                    
                    const int32_t x = flip_x ? source_width - (int)t.i13 + draw_offset_int.x - 1: (int)t.i13 - draw_offset_int.x;
                    const int32_t y = flip_y ? source_height - (int)t.i23 + draw_offset_int.y - 1 : (int)t.i23 - draw_offset_int.y;
                    
                    if (x < 0 || x >= source_width || y < 0 || y >= source_height) {
                        continue;
                    }
                    int32_t i_index = (x + source_origin_x + (y + source_origin_y) * source_data_width) * source_channels;
                    if (image_buffer[i_index + source_alpha_offset] < 128) {
                        continue;
                    }
                    
                    int32_t t_index = (i + y_t_index) * target_channels;
                    target[t_index] = !invert * image_buffer[i_index] + invert * (255 - image_buffer[i_index]);
                }
            }
        }
    } else {
        if (stamp) {
            const uint8_t stamp_color = render_options.stamp_color;
            for (int32_t j = i_top; j < i_bottom; j++) {
                const int32_t y_t_index = j * target_width;

                for (int32_t i = i_left; i < i_right; i++) {
                    AffineTransformFloat t = faf_faf_multiply(inverse_camera, faf_translate(faf_identity(), (Vector2DFloat){ (Float)i, (Float)j }));
                    
                    const int32_t x = (int)t.i13;
                    const int32_t y = (int)t.i23;
                    
                    if (x < 0 || x >= source_width || y < 0 || y >= source_height) {
                        continue;
                    }
                    int32_t t_index = (i + y_t_index) * target_channels;
                    
                    target[t_index] = stamp_color;
                }
            }
        } else {
            for (int32_t j = i_top; j < i_bottom; j++) {
                const int32_t y_t_index = j * target_width;

                for (int32_t i = i_left; i < i_right; i++) {
                    AffineTransformFloat t = faf_faf_multiply(inverse_camera, faf_translate(faf_identity(), (Vector2DFloat){ (Float)i, (Float)j }));
                    
                    const int32_t x = flip_x ? source_width - (int)t.i13 - 1 : (int)t.i13;
                    const int32_t y = flip_y ? source_height - (int)t.i23 - 1 : (int)t.i23;
                    
                    if (x < 0 || x >= source_width || y < 0 || y >= source_height) {
                        continue;
                    }
                    int32_t i_index = (x + source_origin_x + (y + source_origin_y) * source_data_width) * source_channels;
                    int32_t t_index = (i + y_t_index) * target_channels;
                    
                    target[t_index] = !invert * image_buffer[i_index] + invert * (255 - image_buffer[i_index]);
                }
            }
        }
    }

    context_rect_rendered(context, i_left, i_right - 1, i_top, i_bottom - 1);

#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
}

void context_render_rect_dither(RenderContext *context, const Image *image, const Image *dither_texture, const Vector2DInt position, const Vector2DInt offset, const int flip_flags_xy_image, const int flip_flags_xy_dither)
{
    if (!context || !dither_texture || !image) { return; }
    
#ifdef ENABLE_PROFILER
    profiler_start_segment("Prepare context_render_rect_dither");
#endif

    const int32_t source_width = image->rect.size.width;
    const int32_t source_height = image->rect.size.height;
    const int32_t target_width = context->w_target_buffer->size.width;
    const int32_t target_height = context->w_target_buffer->size.height;
    
    const bool flip_x = flip_flags_xy_image & (1 << 0);
    const bool flip_y = flip_flags_xy_image & (1 << 1);

    const Vector2DInt draw_offset = (Vector2DInt){
        flip_x ? image->original.width - (image->offset.x + image->rect.size.width) : image->offset.x,
        flip_y ? image->original.height - (image->offset.y + image->rect.size.height) : image->offset.y,
    };
    
    if (position.x + draw_offset.x > target_width
        || position.x + draw_offset.x + source_width < 0
        || position.y + draw_offset.y > target_height
        || position.y + draw_offset.y + source_height < 0) {
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
        return;
    }

    const bool flip_x_dither = flip_flags_xy_dither & (1 << 0);
    const bool flip_y_dither = flip_flags_xy_dither & (1 << 1);

    const int32_t source_data_width = image->w_image_data->size.width;
    const ImageBuffer *image_buffer = image->w_image_data->buffer;
    const ImageBuffer *dither_buffer = dither_texture->w_image_data->buffer;
    const int32_t dither_width = dither_texture->rect.size.width;
    const int32_t dither_height = dither_texture->rect.size.height;
    const int32_t target_channels = image_data_channel_count(context->w_target_buffer);
    const int32_t source_channels = image_channel_count(image);
    const int32_t dither_channels = image_channel_count(dither_texture);
    const int32_t source_origin_x = image->rect.origin.x;
    const int32_t source_origin_y = image->rect.origin.y;
    const int32_t dither_origin_x = dither_texture->rect.origin.x;
    const int32_t dither_origin_y = dither_texture->rect.origin.y;
    ImageBuffer *target = context->w_target_buffer->buffer;

    const bool source_has_alpha = image_has_alpha(image);
    const int32_t source_alpha_offset = image_alpha_offset(image);
    
    const int32_t start_x = max(0, -position.x - draw_offset.x);
    const int32_t end_x = min(source_width, target_width - position.x - draw_offset.x);
    const int32_t start_y = max(0, -position.y - draw_offset.y);
    const int32_t end_y = min(source_height, target_height - position.y - draw_offset.y);
    
#ifdef ENABLE_PROFILER
    profiler_end_segment();
    profiler_start_segment("Fill context_render_rect_dither");
#endif
    
    if (source_has_alpha) {
        for (int32_t j = start_y; j < end_y; j++) {
            const int32_t ctx_y = j + position.y + draw_offset.y;
            const int32_t y = flip_y * (source_height - j - 1) + !flip_y * j;
            const int32_t dither_y_value = flip_y_dither * (dither_height - j - 1) + !flip_y_dither * j;;
            const int32_t dither_y = (dither_y_value + offset.y) % dither_height;
            const int32_t y_i_index = (y + source_origin_y) * source_data_width;
            const int32_t y_t_index = ctx_y * target_width;
            const int32_t y_d_index = (dither_y + dither_origin_y) * dither_width;
            for (int32_t i = start_x; i < end_x; i++) {
                const int32_t ctx_x = i + position.x + draw_offset.x;
                const int32_t x = flip_x * (source_width - i - 1) + !flip_x * i;
                const int32_t dither_x_value = flip_x_dither * (dither_width - i - 1) + !flip_x_dither * i;

                const int32_t dither_x = (dither_x_value + offset.x) % dither_width;
                
                const uint32_t i_index = (x + source_origin_x + y_i_index) * source_channels;
                
                if (image_buffer[i_index + source_alpha_offset] < 128) {
                    continue;
                }
                
                const uint32_t t_index = (ctx_x + y_t_index) * target_channels;
                const uint32_t d_index = (dither_x + dither_origin_x + y_d_index) * dither_channels;
                target[t_index] = (255 - image_buffer[i_index] < dither_buffer[d_index]) * 255;
            }
        }
    } else {
        for (int32_t j = start_y; j < end_y; j++) {
            const int32_t ctx_y = j + position.y + draw_offset.y;
            const int32_t y = flip_y * (source_height - j - 1) + !flip_y * j;
            const int32_t dither_y_value = flip_y_dither * (dither_height - j - 1) + !flip_y_dither * j;;
            const int32_t dither_y = (dither_y_value + offset.y) % dither_height;
            const int32_t y_i_index = (y + source_origin_y) * source_data_width;
            const int32_t y_t_index = ctx_y * target_width;
            const int32_t y_d_index = (dither_y + dither_origin_y) * dither_width;
            for (int32_t i = start_x; i < end_x; i++) {
                const int32_t ctx_x = i + position.x + draw_offset.x;
                const int32_t x = flip_x * (source_width - i - 1) + !flip_x * i;
                const int32_t dither_x_value = flip_x_dither * (dither_width - i - 1) + !flip_x_dither * i;

                const int32_t dither_x = (dither_x_value + offset.x) % dither_width;
                
                const uint32_t i_index = (x + source_origin_x + y_i_index) * source_channels;
                const uint32_t t_index = (ctx_x + y_t_index) * target_channels;
                const uint32_t d_index = (dither_x + dither_origin_x + y_d_index) * dither_channels;
                target[t_index] = (255 - image_buffer[i_index] < dither_buffer[d_index]) * 255;
            }
        }
    }

#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
}

void context_render_rect_dither_threshold(RenderContext *context, const uint8_t threshold, const Image *image, const Vector2DInt position, const int flip_flags_xy)
{
    if (!context || !image) { return; }
    
#ifdef ENABLE_PROFILER
    profiler_start_segment("Prepare context_render_rect_dither_threshold");
#endif

    const int32_t source_width = image->rect.size.width;
    const int32_t source_height = image->rect.size.height;
    const int32_t target_width = context->w_target_buffer->size.width;
    const int32_t target_height = context->w_target_buffer->size.height;
    
    const bool flip_x = flip_flags_xy & (1 << 0);
    const bool flip_y = flip_flags_xy & (1 << 1);
    
    if (position.x > target_width
        || position.x + source_width < 0
        || position.y > target_height
        || position.y + source_height < 0) {
#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
        return;
    }

    const int32_t source_data_width = image->w_image_data->size.width;
    const ImageBuffer *dither_buffer = image->w_image_data->buffer;
    const int32_t target_channels = image_data_channel_count(context->w_target_buffer);
    const int32_t source_channels = image_channel_count(image);
    const int32_t source_origin_x = image->rect.origin.x;
    const int32_t source_origin_y = image->rect.origin.y;
    ImageBuffer *target = context->w_target_buffer->buffer;

    const bool source_has_alpha = image_has_alpha(image);
    const int32_t source_alpha_offset = image_alpha_offset(image);
    
    const int32_t start_x = max(0, -position.x);
    const int32_t end_x = min(source_width, target_width - position.x);
    const int32_t start_y = max(0, -position.y);
    const int32_t end_y = min(source_height, target_height - position.y);
    
#ifdef ENABLE_PROFILER
    profiler_end_segment();
    profiler_start_segment("Fill context_render_rect_dither_threshold");
#endif
    
    if (source_has_alpha) {
        for (int32_t j = start_y; j < end_y; j++) {
            const int32_t ctx_y = j + position.y;
            const int32_t y = flip_y * (source_height - j - 1) + !flip_y * j;
            const int32_t y_i_index = (y + source_origin_y) * source_data_width;
            const int32_t y_t_index = ctx_y * target_width;
            for (int32_t i = start_x; i < end_x; i++) {
                const int32_t ctx_x = i + position.x;
                
                const int32_t x = flip_x * (source_width - i - 1) + !flip_x * i;
                const uint32_t i_index = (x + source_origin_x + y_i_index) * source_channels;
                
                if (dither_buffer[i_index + source_alpha_offset] < 128) {
                    continue;
                }
                
                const uint32_t t_index = (ctx_x + y_t_index) * target_channels;
                target[t_index] = (dither_buffer[i_index] > threshold) * 255;
            }
        }
    } else {
        for (int32_t j = start_y; j < end_y; j++) {
            const int32_t ctx_y = j + position.y;
            const int32_t y = flip_y * (source_height - j - 1) + !flip_y * j;
            const int32_t y_i_index = (y + source_origin_y) * source_data_width;
            const int32_t y_t_index = ctx_y * target_width;
            for (int32_t i = start_x; i < end_x; i++) {
                const int32_t ctx_x = i + position.x;
                const int32_t x = flip_x * (source_width - i - 1) + !flip_x * i;

                const uint32_t i_index = (x + source_origin_x + y_i_index) * source_channels;
                const uint32_t t_index = (ctx_x + y_t_index) * target_channels;
                target[t_index] = (dither_buffer[i_index] > threshold) * 255;
            }
        }
    }

#ifdef ENABLE_PROFILER
    profiler_end_segment();
#endif
}
