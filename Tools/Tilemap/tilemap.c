#include "tilemap.h"
#include "image_storage.h"
#include "line_reader.h"
#include "base_object.h"
#include "transforms.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct TileBase {
    BASE_OBJECT;
    char *image_base_name;
    uint8_t collision_layer;
    uint8_t collision_directions;
} TileBase;

void tile_base_destroy(void *object)
{
    TileBase *base = (TileBase *)object;
    platform_free(base->image_base_name);
}

char *tile_base_describe(void *object)
{
    TileBase *base = (TileBase *)object;
    
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "image base name: ");
    sb_append_string(sb, base->image_base_name);
    sb_append_string(sb, ", layer: ");
    sb_append_int(sb, (int)base->collision_layer);
    char *description = sb_get_string(sb);
    
    destroy(sb);
    
    return description;
}

BaseType TileBaseType = { "TileBase", &tile_base_destroy, &tile_base_describe };

TileBase *tile_base_create(const char *image_base_name, uint8_t collision_layer, uint8_t collision_directions)
{
    TileBase *base = platform_calloc(1, sizeof(TileBase));
    if (image_base_name) {
        base->image_base_name = platform_strdup(image_base_name);
    } else {
        base->image_base_name = NULL;
    }
    base->collision_layer = collision_layer;
    base->collision_directions = collision_directions;
    base->w_type = &TileBaseType;
    
    return base;
}

void tile_destroy(void *object)
{
}

char *tile_describe(void *object)
{
    Tile *tile = (Tile *)object;
        
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "image: ");
    sb_append_string(sb, describe(tile->w_image));
    sb_append_string(sb, ", layer: ");
    sb_append_int(sb, (int)tile->collision_layer);
    char *description = sb_get_string(sb);
    
    destroy(sb);
    
    return description;
}

BaseType TileType = { "Tile", &tile_destroy, &tile_describe };


Tile *tile_create_with_type_char(const char *image_name, uint8_t collision_layer, uint8_t collision_directions, char type_char)
{
    Image *image = NULL;
    if (image_name) {
        image = get_image(image_name);
        if (!image) {
            return NULL;
        }
    }
    
    Tile *tile = platform_calloc(1, sizeof(Tile));
    tile->w_type = &TileType;
    tile->collision_layer = collision_layer;
    tile->collision_directions = collision_directions;
    tile->w_image = image;
    tile->type_char = type_char;
    
    return tile;
}

Tile *tile_create(const char *image_name, uint8_t collision_layer, uint8_t collision_directions)
{
    return tile_create_with_type_char(image_name, collision_layer, collision_directions, '\0');
}

void tilemap_render(GameObject *obj, RenderContext *ctx)
{
    TileMap *self = (TileMap *)obj;

    Number anchor_x_translate = -nb_mul(nb_mul(obj->anchor.x, obj->size.width), obj->scale.x);
    Number anchor_y_translate = -nb_mul(nb_mul(obj->anchor.y, obj->size.height), obj->scale.y);

    AffineTransform pos = af_identity();
    
    if (self->rotate_and_scale) {
        pos = af_scale(pos, obj->scale);
        pos = af_translate(pos, (Vector2D){ anchor_x_translate, anchor_y_translate });
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->camera_matrix, pos);
        
        AffineTransform tile_pos;
        
        const Size2D tile_size = self->tile_size;

        for (int32_t y = 0; y < self->map_size.height; ++y) {
            for (int32_t x = 0; x < self->map_size.width; ++x) {
                int32_t index = x + y * self->map_size.width;
                tile_pos = af_translate(af_identity(), (Vector2D){
                    tile_size.width * x,
                    tile_size.height * y
                });
                tile_pos = af_af_multiply(pos, tile_pos);
                ctx->camera_matrix = tile_pos;
                
                Tile *tile = (Tile *)list_get(self->tiles, index);
                context_render(ctx, tile->w_image, 0, false);
            }
        }
    } else {
        pos = af_scale(pos, obj->scale);
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->camera_matrix, pos);

        const Size2D tile_size = self->tile_size;

        for (int32_t y = 0; y < self->map_size.height; ++y) {
            for (int32_t x = 0; x < self->map_size.width; ++x) {
                
                int32_t index = x + y * self->map_size.width;
                Tile *tile = (Tile *)list_get(self->tiles, index);
                
                image_render(ctx, tile->w_image, (Vector2DInt){ nb_to_int(pos.i13 + anchor_x_translate + x * tile_size.width), nb_to_int(pos.i23 + anchor_y_translate + y * tile_size.height) }, 0, false);
            }
        }
        
    }
}

void tilemap_destroy(void *object)
{
    TileMap *tilemap = (TileMap *)object;
    destroy(tilemap->tiles);
    destroy(tilemap->tile_dictionary);
    go_destroy(tilemap);
}

char *tilemap_describe(void *sprite)
{
    return go_describe(sprite);
}

GameObjectType TileMapType = {
    { { "TileMap", &tilemap_destroy, &tilemap_describe } },
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &tilemap_render
};

int32_t hex_char_to_int(char hex_char) {
    if (hex_char >= '0' && hex_char <= '9') {
        return hex_char - '0';
    }
    if (hex_char >= 'a' && hex_char <= 'f') {
        return hex_char - 'a' + 10;
    }
    if (hex_char >= 'A' && hex_char <= 'F') {
        return hex_char - 'A' + 10;
    }
    return 0;
}

struct tile_types_context {
    void *context;
    resource_callback_t file_callback;
    HashTable *w_tile_dictionary;
    char *file_name;
};

typedef enum {
    tmp_none,
    tmp_size,
    tmp_map,
    tmp_objects,
    tmp_tiles
} TileMapPart;

struct tm_c_context {
    TileMap *tilemap;
    void *context;
    tilemap_callback_t tilemap_callback;
    char *file_name;
    TileMapPart current_part;
    bool valid;
};

void tilemap_set_tile_edges(TileMap *tilemap)
{
    StringBuilder *sb = sb_create();
    for (int32_t y = 0; y < tilemap->map_size.height; ++y) {
        for (int32_t x = 0; x < tilemap->map_size.width; ++x) {
            int32_t index = x + y * tilemap->map_size.width;
            Tile *tile = (Tile *)list_get(tilemap->tiles, index);
            
            if (tile->type_char == '\0') {
                continue;
            }
            char key[2] = "\0\0";
            key[0] = tile->type_char;
            
            TileBase *base = hashtable_get(tilemap->tile_dictionary, key);
            if (!base || !base->image_base_name) {
                continue;
            }
            
            sb_clear(sb);
            sb_append_string(sb, base->image_base_name);
            
            Tile *l = tilemap_tile_at(tilemap, x - 1, y);
            Tile *r = tilemap_tile_at(tilemap, x + 1, y);
            Tile *u = tilemap_tile_at(tilemap, x, y - 1);
            Tile *d = tilemap_tile_at(tilemap, x, y + 1);
            
            if (l && l->type_char != tile->type_char) {
                sb_append_string(sb, "l");
            }
            if (r && r->type_char != tile->type_char) {
                sb_append_string(sb, "r");
            }
            if (u && u->type_char != tile->type_char) {
                sb_append_string(sb, "u");
            }
            if (d && d->type_char != tile->type_char) {
                sb_append_string(sb, "d");
            }

            char *image_name = sb_get_string(sb);
            
            Image *image = NULL;
            if (image_name) {
                image = get_image(image_name);
                if (!image) {
                    image = get_image(base->image_base_name);
                }
                if (image) {
                    tile->w_image = image;
                }
            }
            
            platform_free(image_name);
        }
    }
    destroy(sb);
}

void tilemap_create_finish(struct tm_c_context *ctx)
{
    if (!ctx->valid) {
        LOG_ERROR("Not a valid tilemap file: %s", ctx->file_name);
        ctx->tilemap_callback(ctx->file_name, NULL, ctx->context);
        platform_free(ctx->file_name);
        destroy(ctx->tilemap);
        return;
    }
    
    ctx->tilemap->size = (Size2D){
        ctx->tilemap->map_size.width * ctx->tilemap->tile_size.width,
        ctx->tilemap->map_size.height * ctx->tilemap->tile_size.height
    };
    
    tilemap_set_tile_edges(ctx->tilemap);
    
    LOG("Tilemap tile count %llu", list_count(ctx->tilemap->tiles));
    
    ctx->tilemap_callback(ctx->file_name, ctx->tilemap, ctx->context);
    platform_free(ctx->file_name);
}

void read_tilemap_line(const char *line, int32_t row_number, bool last_row, void *context)
{
    struct tm_c_context *ctx = (struct tm_c_context *)context;
    TileMap *tilemap = ctx->tilemap;
    
    const char comment_marker = '#';
    if (line[0] == comment_marker) {
        return;
    }
    
    if (line[0] == '[') {
        if (ctx->current_part == tmp_map && list_count(tilemap->tiles) != tilemap->map_size.width * tilemap->map_size.height) {
            LOG_ERROR("Tilemap map size does not match");
            ctx->valid = false;
            ctx->current_part = tmp_none;
            return;
        }
        if (strcmp(line, "[SIZE]") == 0) {
            ctx->current_part = tmp_size;
        } else if (strcmp(line, "[MAP]") == 0) {
            ctx->current_part = tmp_map;
        } else if (strcmp(line, "[OBJECTS]") == 0) {
            ctx->current_part = tmp_objects;
        } else if (strcmp(line, "[TILES]") == 0) {
            ctx->current_part = tmp_tiles;
        } else {
            LOG_ERROR("Tilemap file %s, line %d: Unknown tilemap part %s", ctx->file_name, row_number, line);
            return;
        }
        return;
    }
    
    if (ctx->current_part == tmp_size) {
        if (sscanf(line, "%dx%d", &tilemap->map_size.width, &tilemap->map_size.height) != 2) {
            LOG_ERROR("Cannot read tilemap size");
            ctx->valid = false;
            return;
        }
        ctx->current_part = tmp_none;
    } else if (ctx->current_part == tmp_map) {
        int32_t i = 0;
        for (char t; (t = line[i]) != '\0'; ++i) {
            char key[2] = "\0\0";
            key[0] = t;
            TileBase *base = hashtable_get(tilemap->tile_dictionary, key);
            if (base) {
                Tile *tile;
                if (base->image_base_name == NULL) {
                    tile = tile_create_with_type_char(NULL, 0, 0, t);
                } else {
                    tile = tile_create_with_type_char(base->image_base_name, base->collision_layer, base->collision_directions, t);
                    
                    if (tilemap->tile_size.width == 0 || tilemap->tile_size.height == 0) {
                        tilemap->tile_size = (Size2D){
                            nb_from_int(tile->w_image->rect.size.width),
                            nb_from_int(tile->w_image->rect.size.height)
                        };
                    } else if (nb_from_int(tile->w_image->rect.size.width) != tilemap->tile_size.width ||
                               nb_from_int(tile->w_image->rect.size.height) != tilemap->tile_size.height) {
                        LOG_ERROR("Tilemap tile images are of different size");
                        ctx->valid = false;
                    }
                }
                list_add(tilemap->tiles, tile);
            } else {
                LOG_ERROR("No tile type found for key %s", key);
                ctx->valid = false;
            }
        }
        if (i != tilemap->map_size.width) {
            LOG_ERROR("Tilemap row %d length is wrong", row_number);
            ctx->valid = false;
        }
    } else if (ctx->current_part == tmp_tiles) {
        
        const size_t delimeter_count = 1;
        const char *delimeters = " ";
        
        int token_length = 0;
        int token_start = 0;
        
        ArrayList *tokens = list_create_with_destructor(&platform_free);
        
        size_t length = strlen(line);
        
        for (int32_t i = 0; i < length; ++i) {
            char chr = line[i];
            bool delimeter_found = false;
            for (int32_t j = 0; j < delimeter_count; ++j) {
                if (chr == delimeters[j]) {
                    delimeter_found = true;
                    break;
                }
            }
            if (!delimeter_found && i == length - 1) {
                delimeter_found = true;
                ++token_length;
            }
            
            if (delimeter_found) {
                if (token_length > 0) {
                    char *found_token = platform_strndup(line + token_start, token_length);
                    if (found_token) {
                        list_add(tokens, found_token);
                    }
                }
                token_length = 0;
                token_start = i + 1;
            } else {
                ++token_length;
            }
        }
        size_t token_count = list_count(tokens);
        
        char *token_array[token_count];
        if (token_count > 0) {
            for (size_t i = 0; i < token_count; ++i) {
                token_array[i] = list_get(tokens, i);
            }
        } else if (token_count == 0) {
            destroy(tokens);
            return;
        } else {
            LOG_ERROR("Tilemap file %s, line %d: Cannot read tilemap type %s", ctx->file_name, row_number, line);
            destroy(tokens);
            return;
        }
                    
        if (token_count != 4
            || strlen(token_array[0]) != 1
            || strlen(token_array[3]) != 4)
        {
            LOG_ERROR("Tilemap file %s, line %d: Cannot read tilemap type %s", ctx->file_name, row_number, line);
            destroy(tokens);
            return;
        }
            
        char *image_base_name = strcmp(token_array[1], "$clear") == 0 ? NULL : token_array[1];
        
        uint8_t collision_layer = (uint8_t)atoi(token_array[2]);
        
        uint8_t collision_directions = 0;
        collision_directions += token_array[3][0] == '1' ? (1 << 0) : 0;
        collision_directions += token_array[3][1] == '1' ? (1 << 1) : 0;
        collision_directions += token_array[3][2] == '1' ? (1 << 2) : 0;
        collision_directions += token_array[3][3] == '1' ? (1 << 3) : 0;
        
        TileBase *base = tile_base_create(image_base_name, collision_layer, collision_directions);
        hashtable_put(tilemap->tile_dictionary, token_array[0], base);
        destroy(tokens);
    }
    
    if (last_row) {
        tilemap_create_finish(ctx);
    }
}

void tilemap_create(const char *tilemap_file_name, tilemap_callback_t tilemap_callback, void *context)
{
    GameObject *go = go_alloc(sizeof(TileMap));
    TileMap *tilemap = (TileMap *)go;
    tilemap->w_type = &TileMapType;
    tilemap->tiles = list_create();
    tilemap->tile_dictionary = hashtable_create();
    tilemap->rotate_and_scale = false;
    
    struct tm_c_context *ctx = platform_calloc(1, sizeof(struct tm_c_context));
    ctx->context = context;
    ctx->tilemap = tilemap;
    ctx->tilemap_callback = tilemap_callback;
    ctx->file_name = platform_strdup(tilemap_file_name);
    ctx->current_part = tmp_none;
    ctx->valid = true;
    
    file_read_lines(tilemap_file_name, &read_tilemap_line, ctx);
}

Tile *tilemap_tile_at(TileMap *tilemap, const int32_t x, const int32_t y)
{
    if (x < 0 || y < 0 ||
        x >= tilemap->map_size.width || y >= tilemap->map_size.height) {
        return NULL;
    }
    
    int32_t index = x + y * tilemap->map_size.width;
    return (Tile *)list_get(tilemap->tiles, index);
}
