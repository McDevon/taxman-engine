#ifndef nine_sprite_h
#define nine_sprite_h

#include "game_object.h"
#include "types.h"

struct ns_private;

#define NINE_SPRITE_CONTENTS \
    GAME_OBJECT; \
    struct ns_private *ns_private; \
    Image *w_image; \
    bool invert

typedef struct NineSprite {
    NINE_SPRITE_CONTENTS;
} NineSprite;

#define NINE_SPRITE union { \
    struct { NINE_SPRITE_CONTENTS; }; \
NineSprite sprite_base; \
}

extern GameObjectType NineSpriteType;

NineSprite *nine_sprite_create(const char *image_name, int32_t x_left_split, int32_t x_right_split, int32_t y_upper_split, int32_t y_lower_split);

void nine_sprite_set_image(NineSprite *nine_sprite, Image *image, int32_t x_left_split, int32_t x_right_split, int32_t y_high_split, int32_t y_low_split);
void nine_sprite_set_size(NineSprite *nine_sprite, Size2D size);

#endif /* nine_sprite_h */
