#include "engine_log.h"
#include "hash_table.h"
#include "image.h"
#include "image_storage.h"
#include <stdlib.h>
#include <string.h>
#include "platform_adapter.h"
#include "hash_table_private.h"
#include "string_builder.h"

static HashTableEntry *image_data_table_entry[HASHSIZE];
static HashTable image_data_table = { { { &HashTableType } }, image_data_table_entry };

static HashTableEntry *image_slice_table_entry[HASHSIZE];
static HashTable image_slice_table = { { { &HashTableType } }, image_slice_table_entry };

static HashTableEntry *grid_atlas_table_entry[HASHSIZE];
static HashTable grid_atlas_table = { { { &HashTableType } }, grid_atlas_table_entry };

void load_image_data(const char *image_data_name, const Bool alpha, const Bool make_image)
{
    uint32_t width;
    uint32_t height;
    Bool source_has_alpha;
    ImageBuffer *buffer = NULL;
    platform_load_image(image_data_name, &width, &height, &source_has_alpha, &buffer);
    LOG("Load image data %s w: %d h: %d", image_data_name, width, height);
    if (buffer) {
        int32_t channels = alpha ? 2 : 1;
        int32_t source_channels = source_has_alpha ? 2 : 1;
        ImageData *image_data = image_data_create(platform_calloc(width * height * channels, sizeof(uint8_t)), (Size2DInt){ width, height }, alpha ? image_settings_alpha : 0);
        
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                long source_i = source_channels * (i + j * width);
                long target_i = channels * (i + j * width);
                image_data->buffer[target_i] = buffer[source_i];
                if (alpha) {
                    if (source_has_alpha) {
                        image_data->buffer[target_i + 1] = buffer[source_i + 1];
                    } else {
                        image_data->buffer[target_i + 1] = 0xff;
                    }
                }
            }
        }
        platform_destroy_image(buffer);
        hashtable_put(&image_data_table, image_data_name, image_data);
        if (make_image) {
            hashtable_put(&image_slice_table, image_data_name, image_from_data(image_data));
        }
    }
}

ImageData *get_image_data(const char *image_data_name)
{
    ImageData *entry = hashtable_get(&image_data_table, image_data_name);
    if (!entry) {
        LOG_ERROR("ImageData entry '%s' not found", image_data_name);
        return NULL;
    }
    return entry;
}

void load_grid_atlas(const char *image_data_name, const Bool alpha, const Size2DInt item_size)
{
    load_image_data(image_data_name, alpha, False);
    GridAtlas *atlas = grid_atlas_create(get_image_data(image_data_name), item_size);
    hashtable_put(&grid_atlas_table, image_data_name, atlas);
}

GridAtlas *get_grid_atlas(const char *atlas_name)
{
    GridAtlas *entry = hashtable_get(&grid_atlas_table, atlas_name);
    if (!entry) {
        LOG_ERROR("GridAtlas entry '%s' not found", atlas_name);
        return NULL;
    }
    return entry;
}

Image *image_slice_create_and_store(const char *image_data_name, const char *image_name, const Rect2DInt rect, const Size2DInt original, const Vector2DInt offset)
{
    ImageData *image_data = get_image_data(image_data_name);
    if (!image_data) {
        LOG_ERROR("Cannot create image, image data '%s' not found", image_data_name);
        return NULL;
    }
    
    Image *image = image_create_trimmed(image_data, rect, original, offset);
    if (!image) {
        return NULL;
    }
    LOG("Create image slice %s of %s x: %d y: %d w: %d h: %d", image_name, image_data_name, rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);

    hashtable_put(&image_slice_table, image_name, image);
    
    return image;
}

Image *get_image(const char *image_name)
{
    Image *entry = hashtable_get(&image_slice_table, image_name);
    if (!entry) {
        LOG_ERROR("Image entry '%s' not found", image_name);
        return NULL;
    }
    return entry;
}

void load_sprite_sheet(const char *sprite_sheet_name, const Bool alpha)
{
    StringBuilder *sb = sb_create();
    sb_append_string(sb, sprite_sheet_name);
    sb_append_string(sb, ".txt");
    char *file_name = sb_get_string(sb);
    destroy(sb);

    char *sheet_data = platform_read_text_file(file_name);
    platform_free(file_name);
    
    int row = 0;
    int row_length = 0;
    int row_start = 0;
    int sprite_row = -1;
    Vector2DInt origin = (Vector2DInt){ 0, 0 };
    Size2DInt size = (Size2DInt){ 0, 0 };
    Size2DInt original = (Size2DInt){ 0, 0 };
    Vector2DInt offset = (Vector2DInt){ 0, 0 };

    char sprite_name[30];
    char sprite_sheet_data_name[30];
    char chr;
    
    for (int32_t i = 0; (chr = sheet_data[i]) != '\0'; ++i) {
        if (chr == '\n') {
            if (row == 0) {
                strncpy(sprite_sheet_data_name, sheet_data + row_start, row_length);
                sprite_sheet_data_name[row_length] = '\0';
                load_image_data(sprite_sheet_data_name, alpha, True);
            } else if (row == 5 || sprite_row == 7) {
                sprite_row = 0;
                strncpy(sprite_name, sheet_data + row_start, row_length);
                sprite_name[row_length] = '\0';
            } else if (sprite_row == 2) {
                if (sscanf(sheet_data + row_start, "  xy: %d, %d", &origin.x, &origin.y) != 2) {
                    LOG_ERROR("Cannot read sprite origin for sprite '%s' in sprite sheet '%s'.", sprite_name, sprite_sheet_name);
                    break;
                }
            } else if (sprite_row == 3) {
                if (sscanf(sheet_data + row_start, "  size: %d, %d", &size.width, &size.height) != 2) {
                    LOG_ERROR("Cannot read sprite size for sprite '%s' in sprite sheet '%s'.", sprite_name, sprite_sheet_name);
                    break;
                }
            } else if (sprite_row == 4) {
                if (sscanf(sheet_data + row_start, "  orig: %d, %d", &original.width, &original.height) != 2) {
                    LOG_ERROR("Cannot read sprite original size for sprite '%s' in sprite sheet '%s'.", sprite_name, sprite_sheet_name);
                    break;
                }
            } else if (sprite_row == 5) {
                if (sscanf(sheet_data + row_start, "  offset: %d, %d", &offset.x, &offset.y) != 2) {
                    LOG_ERROR("Cannot read sprite offset for sprite '%s' in sprite sheet '%s'.", sprite_name, sprite_sheet_name);
                    break;
                }
                image_slice_create_and_store(sprite_sheet_data_name, sprite_name, (Rect2DInt){ origin, size }, original, offset);
            }
            
            row_length = 0;
            row_start = i + 1;
            ++row;
            if (sprite_row >= 0) {
                ++sprite_row;
            }
        } else {
            ++row_length;
        }
    }
    
    platform_close_text_file(sheet_data);
}
