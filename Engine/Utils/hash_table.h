#ifndef HashTable_h
#define HashTable_h

#include "types.h"
#include "base_object.h"

#define HASHSIZE 101

extern BaseType HashTableType;
typedef struct HashTableEntry HashTableEntry;
typedef struct HashTable HashTable;

HashTable *hashtable_create(void);
void *hashtable_get(const HashTable *table, const char *key);
int hashtable_put(HashTable *table, const char *key, void *value);

size_t hashtable_count(HashTable *table);

void *hashtable_any(const HashTable *table);

#endif /* HashTable_h */
