#include "render_texture.h"

#include "string_builder.h"
#include "platform_adapter.h"
#include "game_object_private.h"
#include "transforms.h"
#include "image_render.h"

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
    image->offset = (Vector2DInt){ left, top };
}

RenderTexture *render_texture_create_with_rotated(Image *original_image, Number angle)
{
    Vector2D anchor = vec(original_image->rect.size.width / 2, original_image->rect.size.height / 2);
    Vector2D left_up = (Vector2D){ 0, 0 };
    Vector2D right_up = (Vector2D){ nb_from_int(original_image->rect.size.width), 0 };
    Vector2D left_down = (Vector2D){ 0, nb_from_int(original_image->rect.size.height) };
    Vector2D right_down = (Vector2D){ nb_from_int(original_image->rect.size.width), nb_from_int(original_image->rect.size.height) };
    
    Vector2D corners[] = { left_up, right_up, left_down, right_down };
    
    Number top = nb_max_value;
    Number left = nb_max_value;
    Number bottom = nb_min_value;
    Number right = nb_min_value;
    
    const Number angle_sin = nb_sin(angle);
    const Number angle_cos = nb_cos(angle);
        
    for (int i = 0; i < 4; ++i) {
        Vector2D corner = vec_vec_subtract(corners[i], anchor);
        
        Vector2D rotated = vec_vec_add(vec(nb_mul(corner.x, angle_cos) - nb_mul(corner.y, angle_sin),
                                           nb_mul(corner.x, angle_sin) + nb_mul(corner.y, angle_cos)), anchor);
        
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
    Size2DInt size = (Size2DInt){ nb_to_int(right - left), nb_to_int(bottom - top) };
    RenderTexture *rt = render_texture_create(size, image_channel_count(original_image));
    
    context_render_rotate_image_b(rt->render_context,
                                original_image,
                                (Vector2DInt){ -nb_to_int(nb_ceil(left)), -nb_to_int(nb_ceil(top)) },
                                angle,
                                anchor,
                                render_options_make(false, false, false),
                                false
                                );
    
    if (image_has_alpha(original_image)) {
        render_texture_trim_image(rt);
    }
    
    return rt;
}
