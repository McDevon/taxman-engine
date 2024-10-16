#ifndef action_animator_h
#define action_animator_h

#include "game_object_component.h"
#include "types.h"

struct ActionObject;

typedef struct ActionObjectType {
    BASE_TYPE;
    void (*start)(struct ActionObject *, GameObject *);
    Float (*update)(struct ActionObject *, GameObject *, Float); // Returns how much time was left unused
    void (*finish)(struct ActionObject *, GameObject *);
} ActionObjectType;

extern GameObjectComponentType ActType;

#define ACTION_OBJECT_CONTENTS \
    BASE_OBJECT; \
	Float length; \
    Float position

typedef struct ActionObject {
    ACTION_OBJECT_CONTENTS;
} ActionObject;

#define ACTION_OBJECT union { \
    struct { ACTION_OBJECT_CONTENTS; }; \
    ActionObject action_base; \
}

typedef struct Act Act;

Act *act_create(ActionObject *);

#endif /* action_animator_h */
