#include "action_constructors.h"
#include "game_object.h"
#include "transforms.h"
#include "platform_adapter.h"
#include "string_builder.h"
#include "utils.h"
#include "array_list.h"

struct ActionCallback {
    ACTION_OBJECT;
    void (*callback)(void *obj, void *context);
    void *context;
};

typedef struct CallbackContextWeakRef {
    BASE_OBJECT;
    void *w_context;
} CallbackContextWeakRef;

typedef struct CallbackContextStrongRef {
    BASE_OBJECT;
    void *context;
} CallbackContextStrongRef;

void callback_context_weakref_destroy(void *obj)
{
}

void callback_context_strongref_destroy(void *obj)
{
    CallbackContextStrongRef *self = (CallbackContextStrongRef *)obj;
    if (self->context) {
        destroy(self->context);
        self->context = NULL;
    }
}

char *callback_context_weakref_describe(void *obj)
{
    return platform_strdup("pltw");
}

char *callback_context_strongref_describe(void *obj)
{
    return platform_strdup("plts");
}

static BaseType CallbackContextWeakRefType = { "CallbackContextWeakRef", &callback_context_weakref_destroy, &callback_context_weakref_describe };

static BaseType CallbackContextStrongRefType = { "CallbackContextStrongRef", &callback_context_strongref_destroy, &callback_context_strongref_describe };

CallbackContextWeakRef *callback_context_create_weakref(void *context)
{
    CallbackContextWeakRef *object = platform_calloc(1, sizeof(CallbackContextWeakRef));
    object->w_context = context;
    object->w_type = &CallbackContextWeakRefType;
    
    return object;
}

CallbackContextStrongRef *callback_context_create_strongref(void *context)
{
    CallbackContextStrongRef *object = platform_calloc(1, sizeof(CallbackContextStrongRef));
    object->context = context;
    object->w_type = &CallbackContextStrongRefType;
    
    return object;
}

void *callback_context_get(void *context_object)
{
    Object *obj = (Object *)context_object;
    if (obj->w_type == &CallbackContextWeakRefType) {
        CallbackContextWeakRef *weakref = (CallbackContextWeakRef*)obj;
        return weakref->w_context;
    } else if (obj->w_type == &CallbackContextStrongRefType) {
        CallbackContextStrongRef *strongref = (CallbackContextStrongRef*)obj;
        return strongref->context;
    } else {
        return NULL;
    }
}

void action_callback_destroy(void *obj)
{
}

char *action_callback_describe(void *obj)
{
    struct ActionCallback *action = (struct ActionCallback *)obj;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "context: ");
    if (action->context) {
        char *context_desc = describe(action->context);
        sb_append_string(sb, context_desc);
        platform_free(context_desc);
    } else {
        sb_append_string(sb, "NULL");
    }
    
    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

void action_callback_start(ActionObject *action, GameObject *go)
{
    struct ActionCallback *self = (struct ActionCallback*)action;
    self->callback(go, self->context);
}

Float action_callback_update(ActionObject *action, GameObject *go, Float dt_s)
{
    struct ActionCallback *self = (struct ActionCallback*)action;    
    self->position = 1.f;
    
    return dt_s;
}

static ActionObjectType ActionCallbackType = {
    { { "ActionCallback", &action_callback_destroy, &action_callback_describe } },
    &action_callback_start,
    &action_callback_update,
    NULL
};

ActionObject *action_callback_create(void (*callback)(void *obj, void *context), void *context)
{
    struct ActionCallback *object = platform_calloc(1, sizeof(struct ActionCallback));
    object->callback = callback;
    object->context = context;
    object->length = 0.f;
    object->w_type = &ActionCallbackType;

    return (ActionObject *)object;
}

void action_destroy_object_callback(void *obj, void *context)
{
    go_schedule_destroy(obj);
}

ActionObject *action_destroy_create()
{
    return (ActionObject *)action_callback_create(&action_destroy_object_callback, NULL);
}

void action_delayed_callback(void *obj, void *context)
{
    context_callback_t *callback = (context_callback_t *)context;
    callback(obj);
}

void go_delayed_callback(void *obj, context_callback_t *callback, Float length)
{
    GameObject *go = (GameObject *)obj;
    go_add_component(go, act_create(action_sequence_create(({
        ArrayList *list = list_create();
        list_add(list, action_delay_create(length));
        list_add(list, action_callback_create(&action_delayed_callback, callback));
        list;
    }))));
}
