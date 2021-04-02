#include "hash_table.h"
#include "hash_table_private.h"
#include "platform_adapter.h"
#include <stdlib.h>
#include <string.h>

void hashtable_destroy(void *table);
char *hashtable_describe(void *table);

BaseType HashTableType = { "HashTable", &hashtable_destroy, &hashtable_describe };

unsigned hash_string(const char *s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % HASHSIZE;
}

void *hashtable_get(const HashTable *table, const char *key)
{
    HashTableEntry **hashtab = table->entries;
    HashTableEntry *np;
    for (np = hashtab[hash_string(key)]; np != NULL; np = np->next) {
        if (strcmp(key, np->key) == 0) {
            return np->value;
        }
    }
    return NULL;
}

int hashtable_put(HashTable *table, const char *key, void *value)
{
    HashTableEntry **hashtab = table->entries;
    HashTableEntry *np;
    unsigned hashval;
    if ((np = hashtable_get(table, key)) == NULL) {
        np = (HashTableEntry *) platform_malloc(sizeof(*np));
        if (np == NULL || (np->key = platform_strdup(key)) == NULL) {
            return -1;
        }
        hashval = hash_string(key);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    } else {
        destroy(np->value);
    }
    if ((np->value = value) == NULL) {
        return -1;
    }
    return 0;
}

void hashtable_entry_destroy(HashTableEntry *entry)
{
    if (entry->next) {
        hashtable_entry_destroy(entry->next);
        entry->next = NULL;
    }
    destroy(entry->value);
    platform_free(entry->key);
    platform_free(entry);
}

void hashtable_destroy(void *object)
{
    HashTable *table = (HashTable *)object;
    for (size_t i = 0; i < HASHSIZE; ++i) {
        if (table->entries[i] == NULL) {
            continue;
        }
        hashtable_entry_destroy(table->entries[i]);
    }
    platform_free(table->entries);
}

void *hashtable_any(const HashTable *table)
{
    for (size_t i = 0; i < HASHSIZE; ++i) {
        if (table->entries[i] == NULL) {
            continue;
        }
        return table->entries[i]->value;
    }
    return NULL;
}

char *hashtable_describe(void *object)
{
    return platform_strdup("H");
}

HashTable *hashtable_create(void)
{
    HashTable *table = platform_calloc(1, sizeof(HashTable));
    table->w_type = &HashTableType;
    
    void *entries = platform_calloc(HASHSIZE, sizeof(HashTableEntry *));
    table->entries = entries;
    
    return table;
}
