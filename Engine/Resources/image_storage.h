#ifndef file_loader_h
#define file_loader_h

#include <stdio.h>
#include "image_render.h"
#include "grid_atlas.h"
#include "types.h"

void load_image_data(const char *image_data_name, const bool make_image, resource_callback_t resource_callback, void *context);
void load_grid_atlas(const char *image_data_name, const Size2DInt item_size, resource_callback_t resource_callback, void *context);
void load_sprite_sheet(const char *sprite_sheet_name, resource_callback_t resource_callback, void *context);
ImageData *get_image_data(const char *image_data_name);
GridAtlas *get_grid_atlas(const char *atlas_name);
Image *image_slice_create_and_store(const char *image_data_name, const char *image_name, const int start, const Size2DInt size, const Size2DInt original, const Vector2DInt offset);
Image *get_image(const char *image_name);
bool image_exists(const char *image_name);

#endif /* file_loader_h */
