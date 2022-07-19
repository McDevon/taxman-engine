#ifndef tilemap_h
#define tilemap_h

#include "engine.h"

typedef enum {
    tbo_dither,
    tbo_invert,
    tbo_flip_x,
    tbo_flip_y
} TileDrawOption;

typedef struct Tile {
    BASE_OBJECT;
    Image *w_image;
    uint8_t collision_layer;
    uint8_t collision_directions;
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

typedef void (*tilemap_callback_t)(const char *, TileMap *, void *);

Tile *tile_create(const char *image_name, uint8_t collision_layer, uint8_t collision_directions, uint8_t options);
void tilemap_create(const char *tilemap_file_name, tilemap_callback_t tilemap_callback, void *context);

Tile *tilemap_tile_at(TileMap *tilemap, const int32_t x, const int32_t y);

#endif /* tilemap_h */
