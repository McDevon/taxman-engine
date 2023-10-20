#include "render_texture.h"

#include "string_builder.h"
#include "platform_adapter.h"
#include "game_object_private.h"
#include "transforms.h"
#include "image_render.h"
#include <math.h>
#include <float.h>

void render_texture_destroy(void *value)
{
    RenderTexture *self = (RenderTexture *)value;
    
    destroy(self->render_context);
    destroy(self->image);
    destroy(self->image_data);
}

char *render_texture_describe(void *value)
{
    RenderTexture *self = (RenderTexture *)value;

    return sb_string_with_format("Size (%d, %d)", self->image_data->size.width, self->image_data->size.height);
}

BaseType RenderTextureType = { "RenderTexture", &render_texture_destroy, &render_texture_describe };

RenderTexture *render_texture_create(Size2DInt size, int32_t channels)
{
    RenderTexture *rt = platform_calloc(1, sizeof(RenderTexture));
    rt->w_type = &RenderTextureType;
    
    rt->image_data = image_data_create(platform_calloc(size.width * size.height * channels, sizeof(uint8_t)), size, channels == 2 ? image_settings_alpha : 0);
    rt->image = image_from_data(rt->image_data);
    rt->render_context = render_context_create(rt->image_data, false);
    
    return rt;
}

void render_texture_resize(RenderTexture *self, Size2DInt size)
{
    if (size.width == self->image->rect.size.width && size.height == self->image->rect.size.height) {
        return;
    }
    if (size.width <= self->image_data->size.width && size.height <= self->image_data->size.height) {
        // This could check for size.width * size.height <= self->image_data->size.width * self->image_data->size.height
        // and values accordingly to avoid resizing to smaller buffer
        self->image->rect = int_rect_make(0, 0, size.width, size.height);
        self->image->offset = (Vector2DInt){0, 0};
        self->image->original = size;
    } else {
        self->image_data->buffer = platform_realloc(self->image_data->buffer, size.width * size.height * image_data_channel_count(self->image_data) * sizeof(ImageBuffer));
        self->image_data->size = size;
        self->image->rect = int_rect_make(0, 0, size.width, size.height);
        self->image->offset = (Vector2DInt){0, 0};
        self->image->original = size;
    }
}

void render_texture_render_go(RenderTexture *self, GameObject *object)
{
    self->render_context->render_transform = render_camera_get_transform(self->render_context->render_camera);
    go_render(object, self->render_context);
}

void render_texture_trim_image(RenderTexture *self)
{
    Image *image = self->image;
    if (!image_has_alpha(image)) {
        return;
    }
    int32_t width = image->rect.size.width;
    int32_t height = image->rect.size.height;
    int32_t left = width;
    int32_t right = 0;
    int32_t top = height;
    int32_t bottom = 0;
    
    ImageBuffer *buffer = image->w_image_data->buffer;
    
    int32_t alpha_offset = image_alpha_offset(image);
    int32_t source_channels = image_channel_count(image);
    
    for (int32_t y = 0; y < height; ++y) {
        for (int32_t x = 0; x < width; ++x) {
            int32_t index = (x + y * width) * source_channels;
            uint8_t alpha_value = buffer[index + alpha_offset];
            if (alpha_value > 0) {
                if (x < left) {
                    left = x;
                }
                if (y < top) {
                    top = y;
                }
                if (x + 1 > right) {
                    right = x + 1;
                }
                if (y + 1 > bottom) {
                    bottom = y + 1;
                }
            }
        }
    }
    
    image->rect = int_rect_make(left, top, right - left, bottom - top);
    image->offset = (Vector2DInt){ image->offset.x + left, image->offset.y + top };
}

Edges render_texture_get_rotated_edges(Image *original_image, Float angle, Vector2D *in_out_anchor)
{
    const Float offset_x = original_image->offset.x;
    const Float offset_y = original_image->offset.y;
    const Float draw_width = original_image->rect.size.width;
    const Float draw_height = original_image->rect.size.height;
    
    Vector2D left_up = (Vector2D){ offset_x, offset_y };
    Vector2D right_up = (Vector2D){ offset_x + draw_width, offset_y };
    Vector2D left_down = (Vector2D){ offset_x, offset_y + draw_height };
    Vector2D right_down = (Vector2D){ offset_x + draw_width, offset_y + draw_height };

    Vector2D anchor_original = vec(original_image->original.width / 2.f, original_image->original.height / 2.f);
    
    Vector2D corners[] = { left_up, right_up, left_down, right_down };

    Float top = FLT_MAX;
    Float left = FLT_MAX;
    Float bottom = FLT_MIN;
    Float right = FLT_MIN;

    const Float angle_sin = sinf(angle);
    const Float angle_cos = cosf(angle);
        
    for (int i = 0; i < 4; ++i) {
        Vector2D corner = vec_vec_subtract(corners[i], anchor_original);

        Vector2D rotated = vec_vec_add(vec(corner.x * angle_cos - corner.y * angle_sin,
                                           corner.x * angle_sin + corner.y * angle_cos), anchor_original);

        if (rotated.x < left) {
            left = rotated.x;
        }
        if (rotated.x > right) {
            right = rotated.x;
        }
        if (rotated.y < top) {
            top = rotated.y;
        }
        if (rotated.y > bottom) {
            bottom = rotated.y;
        }
    }
    
    if (in_out_anchor) {
        Vector2D anchor_position = *in_out_anchor;
        Vector2D rotation_position = vec_vec_subtract(anchor_position, anchor_original);
        Vector2D rotated_anchor = vec_vec_add(vec(rotation_position.x * angle_cos - rotation_position.y * angle_sin,
                                                  rotation_position.x * angle_sin + rotation_position.y * angle_cos), anchor_original);
        in_out_anchor->x = rotated_anchor.x - left;
        in_out_anchor->y = rotated_anchor.y - top;
    }
    
    return (Edges){ left, right, top, bottom };
}

RenderTexture *render_texture_create_with_rotated(Image *original_image, Float angle) {
    return render_texture_create_with_rotated_anchored(original_image, angle, NULL);
}

RenderTexture *render_texture_create_with_rotated_anchored(Image *original_image, Float angle, Vector2D *in_out_anchor)
{
    Edges rotated_edges = render_texture_get_rotated_edges(original_image, angle, in_out_anchor);
    
    const int32_t i_right = (int32_t)ceilf(rotated_edges.right);
    const int32_t i_bottom = (int32_t)ceilf(rotated_edges.bottom);
    const int32_t i_left = (int32_t)floorf(rotated_edges.left);
    const int32_t i_top = (int32_t)floorf(rotated_edges.top);

    Vector2D anchor_original = vec(original_image->original.width / 2.f, original_image->original.height / 2.f);
    Size2DInt size = (Size2DInt){ i_right - i_left, i_bottom - i_top };
    RenderTexture *rt = render_texture_create(size, image_channel_count(original_image));
    Vector2DInt draw_pos = (Vector2DInt){ -i_left, -i_top };

    context_render_rotate_image(rt->render_context,
                                original_image,
                                draw_pos,
                                angle,
                                anchor_original,
                                render_options_make(false, false, false)
                                );
    
    if (image_has_alpha(original_image)) {
        render_texture_trim_image(rt);
    }
    
    return rt;
}
