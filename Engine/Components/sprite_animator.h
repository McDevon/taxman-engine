#ifndef sprite_animator_h
#define sprite_animator_h

#include "game_object_component.h"
#include "hash_table.h"
#include "sprite.h"

typedef struct AnimationFrame {
    BASE_OBJECT;
    Image *w_image;
    Float frame_time;
} AnimationFrame;

typedef struct Animator {
    GAME_OBJECT_COMPONENT;
    ArrayList *w_current_animation;
    HashTable *animations;
    void (*completion_callback)(struct Animator *obj, void *context);
    void *callback_context;
    int32_t current_frame;
    int32_t repeat_counter;
    Float frame_timer;
} Animator;

extern GameObjectComponentType SpriteAnimationComponentType;

Animator *animator_create(void);

void animator_set_animation(Animator *comp, const char *animation_name);
void animator_set_animation_count(Animator *self, const char *animation_name, int32_t repeat_count, void (*completion_callback)(Animator *obj, void *context), void *context);
void animator_add_animation(Animator *comp, const char *animation_name, ArrayList *frame_list);

AnimationFrame *anim_frame_create(const char *image_name, Float frame_time);
AnimationFrame *anim_frame_create_with_image(Image *image, Float frame_time);

#endif /* sprite_animator_h */
