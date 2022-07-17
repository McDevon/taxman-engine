#ifndef tilemap_h
#define tilemap_h

#include "engine.h"

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
    Size2DInt map_size;
    Size2D tile_size;
    bool rotate_and_scale;
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
