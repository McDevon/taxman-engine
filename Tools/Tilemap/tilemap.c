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
    uint8_t options;
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
    sb_append_string(sb, base->image_base_name ? base->image_base_name : "(NULL)");
    sb_append_string(sb, ", layer: ");
    sb_append_int(sb, (int)base->collision_layer);
    char *description = sb_get_string(sb);
    
    destroy(sb);
    
    return description;
}

BaseType TileBaseType = { "TileBase", &tile_base_destroy, &tile_base_describe };

TileBase *tile_base_create(const char *image_base_name, uint8_t collision_layer, uint8_t collision_directions, uint8_t options)
{
    TileBase *base = platform_calloc(1, sizeof(TileBase));
    if (image_base_name) {
        base->image_base_name = platform_strdup(image_base_name);
    } else {
        base->image_base_name = NULL;
    }
    base->collision_layer = collision_layer;
    base->collision_directions = collision_directions;
    base->options = options;
    base->w_type = &TileBaseType;
    
    return base;
}

void tile_map_object_destroy(void *object)
{
    TileMapObject *to = (TileMapObject *)object;
    destroy(to->attribute_strings);
    platform_free(to->name);
}

char *tile_map_object_describe(void *object)
{
    TileMapObject *to = (TileMapObject *)object;

    return sb_string_with_format("name: %s, position: %d, %d, attribute count: %d", to->name, to->position.x, to->position.y, list_count(to->attribute_strings));
}

BaseType TileMapObjectType = { "TileMapObject", &tile_map_object_destroy, &tile_map_object_describe };

TileMapObject *tile_map_object_create(const char *name, Vector2DInt position, ArrayList *attribute_strings)
{
    TileMapObject *to = platform_calloc(1, sizeof(TileMapObject));
    to->w_type = &TileMapObjectType;
    to->name = platform_strdup(name);
    to->position = position;
    to->attribute_strings = list_create_with_destructor(&platform_free);
    
    size_t count = list_count(attribute_strings);
    for (size_t i = 0; i < count; ++i) {
        list_add(to->attribute_strings, platform_strdup(list_get(attribute_strings, i)));
    }
    
    return to;
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


Tile *tile_create_with_type_char(const char *image_name, uint8_t collision_layer, uint8_t collision_directions, uint8_t options, char type_char)
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
    tile->options = options;
    tile->w_image = image;
    tile->type_char = type_char;
    
    return tile;
}

Tile *tile_create(const char *image_name, uint8_t collision_layer, uint8_t collision_directions, uint8_t options)
{
    return tile_create_with_type_char(image_name, collision_layer, collision_directions, options, '\0');
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
        pos = af_af_multiply(ctx->render_transform, pos);
        
        AffineTransform tile_pos;
        
        const Size2D tile_size = self->tile_size;
        
        if (self->w_dither_mask) {
            LOG_WARNING("Tilemap dither not supported when rotate and scale enabled");
            self->w_dither_mask = NULL;
        }

        for (int32_t y = 0; y < self->map_size.height; ++y) {
            for (int32_t x = 0; x < self->map_size.width; ++x) {
                const int32_t index = x + y * self->map_size.width;
                tile_pos = af_translate(af_identity(), (Vector2D){
                    tile_size.width * x,
                    tile_size.height * y
                });
                tile_pos = af_af_multiply(pos, tile_pos);
                ctx->render_transform = tile_pos;
                
                const Tile *tile = (Tile *)list_get(self->tiles, index);
                const RenderOptions render_options = render_options_make((tile->options & tile_draw_option_flip_x) > 0,
                                                                         (tile->options & tile_draw_option_flip_y) > 0,
                                                                         (tile->options & tile_draw_option_invert) > 0,
                                                                         0, 0);
                context_render(ctx, tile->w_image, render_options);
            }
        }
    } else {
        pos = af_scale(pos, obj->scale);
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->render_transform, pos);

        const Size2D tile_size = self->tile_size;
        const Size2DInt tile_size_int = (Size2DInt){nb_to_int(tile_size.width), nb_to_int(tile_size.height)};
        
        const Number dither_mask_start_x = self->dither_mask_position.x;
        const Number dither_mask_end_x = self->w_dither_mask ? dither_mask_start_x + self->w_dither_mask->size.width : nb_zero;
        const Number dither_mask_start_y = self->dither_mask_position.y;
        const Number dither_mask_end_y = self->w_dither_mask ? dither_mask_start_y + self->w_dither_mask->size.height : nb_zero;

        Image *dither_slice = NULL;
        if (self->w_dither_mask) {
            dither_slice = image_create_trimmed(self->w_dither_mask, (Rect2DInt){{0, 0}, tile_size_int}, tile_size_int, (Vector2DInt){0, 0});
        }

        for (int32_t y = 0; y < self->map_size.height; ++y) {
            for (int32_t x = 0; x < self->map_size.width; ++x) {
                
                const int32_t index = x + y * self->map_size.width;
                const Tile *tile = (Tile *)list_get(self->tiles, index);
                
                const RenderOptions render_options = render_options_make((tile->options & tile_draw_option_flip_x) > 0,
                                                                         (tile->options & tile_draw_option_flip_y) > 0,
                                                                         (tile->options & tile_draw_option_invert) > 0,
                                                                         0, 0);
                
                if (tile->options & tile_draw_option_dither) {
                    Number start_x = nb_mul(nb_from_int(x), tile_size.width);
                    Number end_x = start_x + tile_size.width;
                    Number start_y = nb_mul(nb_from_int(y), tile_size.height);
                    Number end_y = start_y + tile_size.height;
                    const uint8_t flip_flags_dither = (tile->options & tile_draw_option_flip_x ? 0x01 : 0) | (tile->options & tile_draw_option_flip_y ? 0x02 : 0);

                    if (!self->w_dither_mask
                        || start_x < dither_mask_start_x
                        || end_x > dither_mask_end_x
                        || start_y < dither_mask_start_y
                        || end_y > dither_mask_end_y) {
                        
                        context_render_rect_dither_threshold(ctx, self->dither_mask_threshold_color, tile->w_image, (Vector2DInt){ nb_to_int(pos.i13 + anchor_x_translate + x * tile_size.width), nb_to_int(pos.i23 + anchor_y_translate + y * tile_size.height) }, flip_flags_dither);
                    } else {
                        dither_slice->rect = (Rect2DInt){{nb_to_int(start_x - dither_mask_start_x), nb_to_int(start_y - dither_mask_start_y)}, tile_size_int};
                        context_render_rect_dither(ctx, dither_slice, tile->w_image, (Vector2DInt){ nb_to_int(pos.i13 + anchor_x_translate + x * tile_size.width), nb_to_int(pos.i23 + anchor_y_translate + y * tile_size.height) }, (Vector2DInt){0, 0}, 0, flip_flags_dither);
                    }
                } else {
                    context_render_rect_image(ctx, tile->w_image, (Vector2DInt){ nb_to_int(pos.i13 + anchor_x_translate + x * tile_size.width), nb_to_int(pos.i23 + anchor_y_translate + y * tile_size.height) }, render_options);
                }
                
            }
        }
        
    }
}

void tilemap_destroy(void *object)
{
    TileMap *tilemap = (TileMap *)object;
    destroy(tilemap->tile_dictionary);
    destroy(tilemap->data_strings);
    destroy(tilemap->objects);
    destroy(tilemap->tiles);
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

typedef enum {
    tmp_none,
    tmp_size,
    tmp_map,
    tmp_objects,
    tmp_tiles,
    tmp_data
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
            if (image_name && image_exists(image_name)) {
                image = get_image(image_name);
            } else {
                image = get_image(base->image_base_name);
            }
            if (image) {
                tile->w_image = image;
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
    platform_free(ctx);
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
        } else if (strcmp(line, "[DATA]") == 0) {
            ctx->current_part = tmp_data;
        } else {
            LOG_ERROR("Tilemap file %s, line %d: Unknown tilemap part %s", ctx->file_name, row_number, line);
            ctx->current_part = tmp_none;
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
                    tile = tile_create_with_type_char(NULL, 0, 0, 0, t);
                } else {
                    tile = tile_create_with_type_char(base->image_base_name, base->collision_layer, base->collision_directions, base->options, t);
                    
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
        if (i > 0 && i != tilemap->map_size.width) {
            LOG_ERROR("Tilemap row %d length is wrong", row_number);
            ctx->valid = false;
        }
    } else if (ctx->current_part == tmp_tiles) {
        ArrayList *tokens = string_tokenize(line, " ", 1);
        size_t token_count = list_count(tokens);
        
        if (token_count == 0) {
            destroy(tokens);
            return;
        }
        
        if (token_count < 4) {
            LOG_ERROR("Tilemap file %s, line %d: Cannot read tilemap type %s", ctx->file_name, row_number, line);
            destroy(tokens);
            return;
        }
        
        const char *tile_char = list_get(tokens, 0);
        const char *image_name = list_get(tokens, 1);
        const char *collision_str = list_get(tokens, 2);
        const char *collision_dir_str = list_get(tokens, 3);

        if (strlen(tile_char) != 1
            || strlen(collision_dir_str) != 4) {
            LOG_ERROR("Tilemap file %s, line %d: Cannot read tilemap type %s", ctx->file_name, row_number, line);
            destroy(tokens);
            return;
        }
        
        uint8_t options = 0;
        
        for (size_t i = 4; i < token_count; ++i) {
            const char *option = list_get(tokens, i);
            if (strcmp(option, "dither") == 0) {
                options |= tile_draw_option_dither;
            } else if (strcmp(option, "invert") == 0) {
                options |= tile_draw_option_invert;
            } else if (strcmp(option, "flip_x") == 0) {
                options |= tile_draw_option_flip_x;
            } else if (strcmp(option, "flip_y") == 0) {
                options |= tile_draw_option_flip_y;
            } else {
                LOG_ERROR("Tilemap file %s, line %d: Unknown tile option %s", ctx->file_name, row_number, option);
            }
        }
        
        const char *image_base_name = strcmp(image_name, "$clear") == 0 ? NULL : image_name;
        
        uint8_t collision_layer = (uint8_t)atoi(collision_str);
        
        uint8_t collision_directions = 0;
        collision_directions += collision_dir_str[0] == '1' ? (1 << 0) : 0;
        collision_directions += collision_dir_str[1] == '1' ? (1 << 1) : 0;
        collision_directions += collision_dir_str[2] == '1' ? (1 << 2) : 0;
        collision_directions += collision_dir_str[3] == '1' ? (1 << 3) : 0;
        
        TileBase *base = tile_base_create(image_base_name, collision_layer, collision_directions, options);
        hashtable_put(tilemap->tile_dictionary, tile_char, base);
        destroy(tokens);
    } else if (ctx->current_part == tmp_objects) {
        ArrayList *tokens = string_tokenize(line, " ,", 2);
        size_t token_count = list_count(tokens);
        
        if (token_count == 0) {
            destroy(tokens);
            if (last_row) {
                tilemap_create_finish(ctx);
            }
            return;
        }
        
        if (token_count < 3) {
            LOG_ERROR("Tilemap file %s, line %d: Cannot read tilemap object %s", ctx->file_name, row_number, line);
            destroy(tokens);
            if (last_row) {
                tilemap_create_finish(ctx);
            }
            return;
        }
                
        const char *str_object_name = list_get(tokens, 0);
        const char *str_pos_x = list_get(tokens, 1);
        const char *str_pos_y = list_get(tokens, 2);
        
        int32_t pos_x = atoi(str_pos_x);
        int32_t pos_y = atoi(str_pos_y);
        
        ArrayList *attributes = list_create_with_weak_references();
        for (int i = 3; i < token_count; ++i) {
            list_add(attributes, list_get(tokens, i));
        }

        TileMapObject *obj = tile_map_object_create(str_object_name, (Vector2DInt){pos_x, pos_y}, attributes);
        
        destroy(attributes);
        destroy(tokens);
        
        list_add(ctx->tilemap->objects, obj);
    } else if (ctx->current_part == tmp_data) {
        if (strlen(line) > 0) {
            list_add(ctx->tilemap->data_strings, platform_strdup(line));
        }
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
    tilemap->objects = list_create();
    tilemap->data_strings = list_create_with_destructor(&platform_free);
    tilemap->tile_dictionary = hashtable_create();
    tilemap->rotate_and_scale = false;
    tilemap->w_dither_mask = NULL;
    tilemap->dither_mask_position = vec_zero();
    tilemap->dither_mask_threshold_color = 128;
    
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
