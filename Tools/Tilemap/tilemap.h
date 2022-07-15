#ifndef tilemap_h
#define tilemap_h

#include "engine.h"

typedef struct Tile {
    BASE_OBJECT;
    Image *w_image;
    uint8_t collision_layer;
    uint8_t collision_directions;
    char type_char;
} Tile;

typedef struct TileMap {
    GAME_OBJECT;
    ArrayList *tiles;
    HashTable *tile_dictionary;
    Size2DInt map_size;
    Size2D tile_size;
    bool rotate_and_scale;
} TileMap;

extern GameObjectType TileMapType;

typedef void (*tilemap_callback_t)(const char *, TileMap *, void *);

Tile *tile_create(const char *image_name, uint8_t collision_layer, uint8_t collision_directions);
void tilemap_create(const char *tilemap_file_name, tilemap_callback_t tilemap_callback, void *context);

Tile *tilemap_tile_at(TileMap *tilemap, const int32_t x, const int32_t y);

#endif /* tilemap_h */
