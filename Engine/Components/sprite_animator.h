#ifndef sprite_animator_h
#define sprite_animator_h

#include "game_object_component.h"
#include "hash_table.h"
#include "sprite.h"

typedef struct AnimationFrame {
    BASE_OBJECT;
    Image *w_image;
    Number frame_time_ms;
} AnimationFrame;

typedef struct Animator {
    GAME_OBJECT_COMPONENT;
    ArrayList *w_current_animation;
    HashTable *animations;
    int32_t current_frame;
    Number frame_timer;
} Animator;

extern GameObjectComponentType SpriteAnimationComponentType;

Animator *animator_create(void);

void animator_set_animation(Animator *comp, const char *animation_name);
void animator_add_animation(Animator *comp, const char *animation_name, ArrayList *frame_list);

AnimationFrame *anim_frame_create(const char *image_name, Number frame_time);

#endif /* sprite_animator_h */
