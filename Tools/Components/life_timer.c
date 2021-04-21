#include "life_timer.h"

void life_timer_destroy(void *comp)
{
    comp_destroy(comp);
}

char *life_timer_describe(void *comp)
{
    return comp_describe(comp);
}

void life_timer_fixed_update(GameObjectComponent *comp, Number dt_ms)
{
    LifeTimer *self = (LifeTimer *)comp;
    
    if (self->paused) {
        return;
    }
    
    self->timer -= dt_ms;
    if (self->timer < nb_zero) {
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

LifeTimer *life_timer_create(Number time, bool paused)
{
    LifeTimer *timer = (LifeTimer *)comp_alloc(sizeof(LifeTimer));
    
    timer->w_type = &LifeTimerComponentType;
    
    timer->timer = time;
    timer->paused = paused;

    return timer;
}
