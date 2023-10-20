#include "life_timer.h"

void life_timer_destroy(void *comp)
{
    comp_destroy(comp);
}

char *life_timer_describe(void *comp)
{
    return comp_describe(comp);
}

void life_timer_fixed_update(GameObjectComponent *comp, Float dt)
{
    LifeTimer *self = (LifeTimer *)comp;
    
    if (self->paused) {
        return;
    }
    
    self->timer -= dt;
    if (self->timer < fn_zero) {
        go_schedule_destroy(comp_get_parent(self));
    }
}

GameObjectComponentType LifeTimerComponentType = {
    { { "LifeTimer", &life_timer_destroy, &life_timer_describe } },
    NULL,
    NULL,
    NULL,
    NULL,
    &life_timer_fixed_update
};

LifeTimer *life_timer_create(Float time, bool paused)
{
    LifeTimer *timer = (LifeTimer *)comp_alloc(sizeof(LifeTimer));
    
    timer->w_type = &LifeTimerComponentType;
    
    timer->timer = time;
    timer->paused = paused;

    return timer;
}
