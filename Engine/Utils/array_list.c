#include "array_list.h"
#include "engine_log.h"
#include "string_builder.h"
#include "platform_adapter.h"
#include "string_utils.h"
#include <stdio.h>
#include <stdarg.h>

#define DEFAULT_INITIAL_CAPACITY 2

const int list_sorted_ascending = -1;
const int list_sorted_descending = 1;
const int list_sorted_same = 0;

struct ArrayList {
    BASE_OBJECT;
    void **first;
    void (*destructor)(void *);
    size_t count;
    size_t capacity;
};

void list_destroy(void *value);
char *list_describe(void *value);

BaseType ArrayListType = { "ArrayList", &list_destroy, &list_describe };

int list_add(ArrayList *list, void *value)
{
    if (value == NULL) {
        LOG_ERROR("Cannot add NULL to a list");
        return -1;
    }
    
    if (list->count >= list->capacity) {
        void **buffer = list->first;
        size_t new_capacity = list->capacity * 2;
        void *new_buffer = platform_realloc(buffer, sizeof(void *) * new_capacity);
        if (!new_buffer) { return 1; }
        
        list->capacity = new_capacity;
        list->first = new_buffer;
    }

    list->first[list->count] = value;
    list->count++;
    
    return 0;
}

int list_insert(ArrayList *list, void *value, size_t index)
{
    if (value == NULL) {
        LOG_ERROR("Cannot add NULL to a list");
        return -1;
    }
    
    if (index > list->count) {
        LOG_ERROR("Cannot add item to list: index too high");
        return -2;
    }
    
    if (list->count >= list->capacity) {
        void **buffer = list->first;
        size_t new_capacity = list->capacity * 2;
        void *new_buffer = platform_realloc(buffer, sizeof(void *) * new_capacity);
        if (!new_buffer) { return 1; }
        
        list->capacity = new_capacity;
        list->first = new_buffer;
    }
    
    for (int32_t i = (int32_t)list->count - 1; i >= (int32_t)index; --i) {
        list->first[i + 1] = list->first[i];
    }

    list->first[index] = value;
    list->count++;
    
    return 0;
}

void * list_get(ArrayList *list, size_t index)
{
    if (index < 0 || index >= list->count) { return NULL; }
    return list->first[index];
}

void * list_drop_index(ArrayList *list, size_t index)
{
    if (index < 0 || index >= list->count) { return NULL; }
    Object *value = list->first[index];
    
    for (size_t i = index; i < list->count - 1; ++i) {
        list->first[i] = list->first[i+1];
    }
    list->first[list->count - 1] = NULL;
    list->count--;
    
    return value;
}

void * list_drop_item(ArrayList *list, void *value)
{
    for (size_t i = 0; i < list->count; ++i) {
        if (list->first[i] == value) {
            return list_drop_index(list, i);
        }
    }
    return NULL;
}

void list_clear(ArrayList *list)
{
    for (int i = 0; i < list->count; ++i) {
        Object *obj = (Object *)list->first[i];
        if (list->destructor) {
            list->destructor(obj);
        }
        list->first[i] = NULL;
    }
    list->count = 0;
}

inline size_t list_count(ArrayList *list)
{
    return list->count;
}

bool list_contains(ArrayList *list, void *value)
{
    for (int i = 0; i < list->count; ++i) {
        if (list->first[i] == value) {
            return true;
        }
    }
    return false;
}

void list_sort(ArrayList *list, list_compare_t *compare_fn)
{
    qsort(list->first, list->count, sizeof(void *), compare_fn);
}

void list_sort_insertsort(ArrayList *list, list_compare_t *compare_fn)
{
    void *key;
    int32_t j;
    size_t count = list->count;
    void **array = list->first;
    for (size_t i = 1; i < count; i++) {
        key = array[i];
        j = (int32_t)(i - 1);

        while (j >= 0 && compare_fn(&key, &array[j]) < 0) {
            array[j + 1] = array[j];
            j = j - 1;
        }
        array[j + 1] = key;
    }
}

void list_reverse(ArrayList *list)
{
    void *key;
    size_t count = list->count;
    size_t half_count = count / 2;
    void **array = list->first;
    for (size_t i = 1; i < half_count; i++) {
        key = array[i];
        array[i] = array[count - 1 - i];
        array[count - 1 - i] = key;
    }
}

ArrayList *list_create_with_destructor(void (*destructor)(void *))
{
    void *buffer = platform_calloc(DEFAULT_INITIAL_CAPACITY, sizeof(Object *));
    if (!buffer) { return NULL; }
    
    ArrayList *list = platform_calloc(1, sizeof(ArrayList));
    list->capacity = DEFAULT_INITIAL_CAPACITY;
    list->count = 0;
    list->first = buffer;
    list->w_type = &ArrayListType;
    list->destructor = destructor;
    
    return list;
}

ArrayList *__list_of_strings(const char * first, ...)
{
    ArrayList *list = list_create_with_destructor(&platform_free);
        
    va_list arg;
    va_start (arg, first);
    for (const char *string_value = first; string_value != NULL; string_value = va_arg(arg, char *)) {
        list_add(list, platform_strdup(string_value));
    }
    va_end (arg);

    return list;
}

ArrayList *__list_of_objects(void * first, ...)
{
    ArrayList *list = list_create();
        
    va_list arg;
    va_start (arg, first);
    for (void *object = first; object != NULL; object = va_arg(arg, void *)) {
        list_add(list, object);
    }
    va_end (arg);

    return list;
}

bool list_contains_string(ArrayList *list, const char *string)
{
    for (int i = 0; i < list->count; ++i) {
        const char *value = list->first[i];
        if (str_equals(value, string)) {
            return true;
        }
    }
    return false;

}

ArrayList *list_create_with_weak_references()
{
    return list_create_with_destructor(NULL);
}

ArrayList *list_create()
{
    return list_create_with_destructor(&destroy);
}

void list_destroy(void *value)
{
    ArrayList *list = (ArrayList *)value;
    if (list->destructor) {
        for (int i = 0; i < list->count; ++i) {
            Object *obj = (Object *)list->first[i];
            list->destructor(obj);
        }
    }
    platform_free(list->first);
}

char *list_describe(void *value)
{
    ArrayList *list = (ArrayList *)value;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "count: ");
    sb_append_int(sb, (int)list->count);
    sb_append_string(sb, " capacity: ");
    sb_append_int(sb, (int)list->capacity);
    
    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}
