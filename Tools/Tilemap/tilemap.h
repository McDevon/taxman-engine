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
    Size2DInt map_size;
    Size2D tile_size;
    Bool rotate_and_scale;
} TileMap;

extern GameObjectType TileMapType;

void load_tile_types(const char *type_file_name, HashTable *tile_dictionary);
Tile *tile_create(const char *image_name, uint8_t collision_layer, uint8_t collision_directions);
TileMap *tilemap_create(const char *tilemap_file_name, const HashTable *tile_dictionary);

Tile *tilemap_tile_at(TileMap *tilemap, const int32_t x, const int32_t y);

#endif /* tilemap_h */
