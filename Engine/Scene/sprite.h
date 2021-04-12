#ifndef sprite_h
#define sprite_h

#include "game_object.h"

#define SPRITE_CONTENTS \
    GAME_OBJECT; \
    Image *w_image; \
    bool flip_x; \
    bool flip_y; \
    bool rotate_and_scale

typedef struct Sprite {
    SPRITE_CONTENTS;
} Sprite;

#define SPRITE union { \
    struct { SPRITE_CONTENTS; }; \
    Sprite sprite_base; \
}

extern GameObjectType SpriteType;

Sprite *sprite_create(const char *image_name);

#endif /* sprite_h */
