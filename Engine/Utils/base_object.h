#ifndef base_object_h
#define base_object_h

#define BO_CONTENTS const void *w_type

#define BT_CONTENTS char *type_name; \
    object_destroy_fnc *destroy; \
    object_description_fnc *description

struct baseobj;

typedef void (object_destroy_fnc)(void *obj);
typedef char *(object_description_fnc)(void *obj);

typedef struct BaseType {
    BT_CONTENTS;
} BaseType;

typedef struct Object {
    BO_CONTENTS;
} Object;

#define BASE_OBJECT union { \
    struct { BO_CONTENTS; }; \
    Object base; \
}

#define BASE_TYPE union { \
    struct { BT_CONTENTS; }; \
    BaseType baseType; \
}

void destroy(void *object);
char *object_type_string(void *object);
char *describe(void *object);
char *describe_debug(void *object);
void describe_debug_to_log(void *object);

#endif /* base_object_h */
