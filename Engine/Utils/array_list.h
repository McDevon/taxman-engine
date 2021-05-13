#ifndef array_list_h
#define array_list_h

#include "types.h"
#include "base_object.h"

extern const int list_sorted_ascending;
extern const int list_sorted_descending;
extern const int list_sorted_same;

extern BaseType ArrayListType;
typedef struct ArrayList ArrayList;

typedef int (list_compare_t)(const void *, const void *);

int list_add(ArrayList *list, void *value);
int list_insert(ArrayList *list, void *value, size_t index);
void * list_get(ArrayList *list, size_t index);
void * list_drop_index(ArrayList *list, size_t index);
void * list_drop_item(ArrayList *list, void *value);

void list_clear(ArrayList *list);

size_t list_count(ArrayList *list);
bool list_contains(ArrayList *list, void *value);

void list_sort(ArrayList *list, list_compare_t *compare_fn);
void list_sort_insertsort(ArrayList *list, list_compare_t *compare_fn);
void list_reverse(ArrayList *list);

ArrayList *list_create(void);
ArrayList *list_create_with_weak_references(void);
ArrayList *list_create_with_destructor(void (*destructor)(void *));

#endif /* array_list_h */
