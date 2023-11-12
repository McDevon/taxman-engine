#ifndef tilemap_h
#define tilemap_h

#include "engine.h"

#define tile_draw_option_flip_x 0x01
#define tile_draw_option_flip_y 0x02
#define tile_draw_option_invert 0x04
#define tile_draw_option_dither 0x08

typedef struct Tile {
    BASE_OBJECT;
    Image *w_image;
    DirectionTable collision_directions;
    uint8_t collision_layer;
    uint8_t options;
    char type_char;
} Tile;

typedef struct TileMap {
    GAME_OBJECT;
    ArrayList *tiles;
    ArrayList *objects;
    ArrayList *data_strings;
    HashTable *tile_dictionary;
    ImageData *w_dither_mask;
    Vector2D dither_mask_position;
    Size2DInt map_size;
    Size2D tile_size;
    bool rotate_and_scale;
    uint8_t dither_mask_threshold_color;
} TileMap;

typedef struct TileMapObject {
    BASE_OBJECT;
    char *name;
    ArrayList *attribute_strings;
    Vector2DInt position;
} TileMapObject;

extern GameObjectType TileMapType;

typedef void (tilemap_callback_t)(const char *, TileMap *, void *);

Tile *tile_create(const char *image_name, uint8_t collision_layer, DirectionTable collision_directions, uint8_t options);
void tilemap_create(const char *tilemap_file_name, tilemap_callback_t tilemap_callback, void *context);

Tile *tilemap_tile_at(TileMap *tilemap, const int32_t x, const int32_t y);

#endif /* tilemap_h */
