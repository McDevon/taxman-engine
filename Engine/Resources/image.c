#include "image.h"
#include "types.h"
#include "engine_log.h"
#include "string_builder.h"
#include "transforms.h"
#include "platform_adapter.h"
#include <stdlib.h>

void image_data_destroy(void *value)
{
    ImageData *image = (ImageData *)value;
    if (image->buffer) {
        platform_free(image->buffer);
    }
}

char *image_data_describe(void *value)
{
    ImageData *image = (ImageData *)value;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "size: ");
    sb_append_int_size(sb, image->size);
    
    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

void image_data_clear(ImageData *image)
{
    if (!image) { return; }
    
    const uint32_t image_width = image->size.width;
    for (int32_t i = 0; i < image_width; i++) {
        for (int32_t j = 0; j < image->size.height; j++) {
            int32_t i_index = (i + j * image_width) * 4;
            image->buffer[i_index] = 0;
        }
    }
}

BaseType ImageDataType = { "ImageData", &image_data_destroy, &image_data_describe };

ImageData *image_data_create(ImageBuffer *buffer, const Size2DInt size, const uint32_t settings)
{
    ImageData *image = platform_calloc(1, sizeof(ImageData));
    image->buffer = buffer;
    image->size = size;
    image->settings = settings;
    
    image->w_type = &ImageDataType;
    
    return image;
}

static inline uint32_t image_settings_channel_count(uint32_t settings)
{
    return 1 + (settings & image_settings_alpha) + (settings & image_settings_rgb);
}

ImageData *image_data_xor_texture(const Size2DInt size, const Vector2DInt offset, const uint32_t settings)
{
    uint32_t channels = image_settings_channel_count(settings);
    ImageData *image_data = image_data_create(platform_calloc(size.width * size.height * channels, sizeof(uint8_t)), size, settings);
    bool colors = settings & image_settings_rgb;
    bool alpha = settings & image_settings_alpha;
    for (int j = 0; j < size.height; j++) {
        for (int i = 0; i < size.width; i++) {
            long index = channels * (i + j * size.width);
            long value = (((i + offset.x) ^ (j + offset.y)) % 256);
            image_data->buffer[index] = (uint8_t)value;
            if (colors) {
                image_data->buffer[++index] = (uint8_t)value;
                image_data->buffer[++index] = (uint8_t)value;
            }
            if (alpha) {
                image_data->buffer[++index] = 255;
            }
        }
    }
    return image_data;
}

uint32_t image_data_channel_count(const ImageData *image)
{
    return image_settings_channel_count(image->settings);
}

uint32_t image_channel_count(const Image *image)
{
    return image_settings_channel_count(image->w_image_data->settings);
}

bool image_data_has_alpha(const ImageData *image)
{
    return image->settings & image_settings_alpha;
}

bool image_has_alpha(const Image *image)
{
    return image->w_image_data->settings & image_settings_alpha;
}

int32_t image_data_alpha_offset(const ImageData *image)
{
    return image->settings & image_settings_alpha
    ? image_settings_channel_count(image->settings) - 1
    : 0;
}

int32_t image_alpha_offset(const Image *image)
{
    return image->w_image_data->settings & image_settings_alpha
    ? image_settings_channel_count(image->w_image_data->settings) - 1
    : 0;
}

void image_destroy(void *value)
{
}

char *image_describe(void *value)
{
    Image *image = (Image *)value;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "source: ");
    char *source_description = describe(image->w_image_data);
    sb_append_string(sb, source_description);
    sb_append_string(sb, " rect: ");
    sb_append_int_rect(sb, image->rect);
    
    char *description = sb_get_string(sb);
    platform_free(source_description);
    destroy(sb);

    return description;
}

BaseType ImageType = { "Image(Slice)", &image_destroy, &image_describe };

Image *image_create_trimmed(ImageData *w_image_data, const Rect2DInt rect, const Size2DInt original, const Vector2DInt offset)
{
    if (rect.origin.x < 0 || rect.origin.y < 0
        || rect.origin.x + rect.size.width > w_image_data->size.width
        || rect.origin.y + rect.size.height > w_image_data->size.height) {
        LOG_ERROR("Image rect outside source data");
        return NULL;
    }
    
    Image *image = platform_calloc(1, sizeof(Image));
    image->w_image_data = w_image_data;
    image->rect = rect;
    image->offset = offset;
    image->original = original;
    
    image->w_type = &ImageType;
    
    return image;
}

Image *image_create(ImageData *w_image_data, const Rect2DInt rect)
{
    return image_create_trimmed(w_image_data, rect, rect.size, (Vector2DInt){ 0, 0 });
}

Image *image_from_data(ImageData *w_image_data)
{
    return image_create(w_image_data, int_rect_make(0, 0, w_image_data->size.width, w_image_data->size.height));
}
