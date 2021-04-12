#ifndef image_h
#define image_h

#include "base_object.h"
#include "types.h"
#include <stdio.h>

#define image_settings_alpha 1
#define image_settings_rgb 2

typedef uint8_t ImageBuffer;

typedef struct ImageData {
    BASE_OBJECT;
    ImageBuffer *buffer;
    Size2DInt size;
    uint32_t settings;
} ImageData;

typedef struct Image {
    BASE_OBJECT;
    ImageData *w_image_data;
    Rect2DInt rect;
    Size2DInt original;
    Vector2DInt offset;
} Image;

ImageData *image_data_create(ImageBuffer *buffer, const Size2DInt size, const uint32_t settings);
ImageData *image_data_xor_texture(const Size2DInt size, const Vector2DInt offset, const uint32_t settings);
void image_data_clear(ImageData *image);

uint32_t image_data_channel_count(const ImageData *image);
uint32_t image_channel_count(const Image *image);
bool image_data_has_alpha(const ImageData *image);
bool image_has_alpha(const Image *image);
int32_t image_data_alpha_offset(const ImageData *image);
int32_t image_alpha_offset(const Image *image);

Image *image_from_data(ImageData *w_image_data);
Image *image_create(ImageData *w_image_data, const Rect2DInt rect);
Image *image_create_trimmed(ImageData *w_image_data, const Rect2DInt rect, const Size2DInt original, const Vector2DInt offset);

#endif /* image_h */
