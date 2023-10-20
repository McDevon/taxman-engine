#ifndef life_timer_h
#define life_timer_h

#include "engine.h"

typedef struct LifeTimer {
    GAME_OBJECT_COMPONENT;
    FixNumber timer;
    bool paused;
} LifeTimer;

extern GameObjectComponentType LifeTimerComponentType;

LifeTimer *life_timer_create(FixNumber time, bool paused);

#endif /* life_timer_h */
