#include "sprite_animator.h"
#include "hash_table.h"
#include "image_storage.h"
#include "platform_adapter.h"

void anim_frame_destroy(void *value)
{
}

char *anim_frame_describe(void *value)
{
    return platform_strdup("frm");
}

BaseType AnimationFrameType = { "AnimationFrame", &anim_frame_destroy, &anim_frame_describe };

AnimationFrame *anim_frame_create(const char *image_name, Float frame_time)
{
    Image *image = get_image(image_name);
    if (!image) {
        return NULL;
    }
    
    AnimationFrame *frame = platform_calloc(1, sizeof(AnimationFrame));
    frame->w_type = &AnimationFrameType;
    frame->w_image = image;
    frame->frame_time = frame_time;
    
    return frame;
}

void animator_destroy(void *comp)
{
    Animator *anim = (Animator *)comp;
    comp_destroy(comp);
    
    destroy(anim->animations);
}

char *animator_describe(void *comp)
{
    return comp_describe(comp);
}

void animator_added(GameObjectComponent *comp)
{

}

void animator_set_current_frame(Animator *self)
{
    if (!self->w_current_animation) {
        return;
    }
    Sprite *sprite = (Sprite*)comp_get_parent(self);
    AnimationFrame *frame = list_get(self->w_current_animation, self->current_frame);
    
    sprite_set_image(sprite, frame->w_image);
    self->frame_timer += frame->frame_time;
}

void animator_start(GameObjectComponent *comp)
{
    Animator *self = (Animator *)comp;
    if (self->w_current_animation == NULL) {
        self->w_current_animation = hashtable_any(self->animations);
    }
    self->current_frame = 0;
    self->frame_timer = 0.f;
    
    animator_set_current_frame(self);
}

void animator_update(GameObjectComponent *comp, Float dt)
{
    Animator *self = (Animator *)comp;
    
    if (!self->w_current_animation) {
        return;
    }
    
    if (self->repeat_counter == 0) {
        return;
    }
    
    size_t frame_count = list_count(self->w_current_animation);
    if (frame_count <= 1) {
        return;
    }
    
    self->frame_timer -= dt;
    
    if (self->frame_timer <= 0.f) {
        if (self->current_frame + 1 >= list_count(self->w_current_animation)) {
            if (self->repeat_counter > 0) {
                self->repeat_counter -= 1;
            }
            if (self->repeat_counter != 0) {
                self->current_frame = 0;
            } else {
                if (self->completion_callback) {
                    self->completion_callback(self, self->callback_context);
                }
            }
        } else {
            self->current_frame++;
        }
        animator_set_current_frame(self);
    }
}


GameObjectComponentType SpriteAnimationComponentType = {
    { { "SpriteAnimationComponent", &animator_destroy, &animator_describe } },
    &animator_added,
    NULL,
    &animator_start,
    &animator_update,
    NULL
};

Animator *animator_create()
{
    Animator *anim = (Animator *)comp_alloc(sizeof(Animator));
    
    anim->w_type = &SpriteAnimationComponentType;
    anim->animations = hashtable_create();
    
    return anim;
}

void animator_set_animation_count(Animator *self, const char *animation_name, int32_t repeat_count, void (*completion_callback)(Animator *obj, void *context), void *context)
{
    ArrayList *target_animation = hashtable_get(self->animations, animation_name);
    if (!target_animation || self->w_current_animation == target_animation) {
        return;
    }
    self->w_current_animation = target_animation;
    self->current_frame = 0;
    self->frame_timer = 0.f;
    self->repeat_counter = repeat_count;
    self->completion_callback = completion_callback;
    self->callback_context = context;
    animator_set_current_frame(self);
}

void animator_set_animation(Animator *self, const char *animation_name)
{
    animator_set_animation_count(self, animation_name, -1, NULL, NULL);
}

void animator_add_animation(Animator *self, const char *animation_name, ArrayList *frame_list)
{
    hashtable_put(self->animations, animation_name, frame_list);
}
