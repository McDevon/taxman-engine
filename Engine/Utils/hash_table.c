#include "hash_table.h"
#include "hash_table_private.h"
#include "platform_adapter.h"
#include "string_builder.h"
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
        if (np->value && table->destructor) {
            table->destructor(np->value);
        }
    }
    if ((np->value = value) == NULL) {
        return -1;
    }
    return 0;
}

bool hashtable_contains(HashTable *table, const char *key)
{
    HashTableEntry **hashtab = table->entries;
    HashTableEntry *np;
    for (np = hashtab[hash_string(key)]; np != NULL; np = np->next) {
        if (strcmp(key, np->key) == 0) {
            return true;
        }
    }
    return false;
}

int hashtable_remove(HashTable *table, const char *key)
{
    HashTableEntry **hashtab = table->entries;
    HashTableEntry *np;
    HashTableEntry *previous = NULL;
    
    for (np = hashtab[hash_string(key)]; np != NULL; np = np->next) {
        if (strcmp(key, np->key) == 0) {
            break;
        }
        previous = np;
    }
    
    if (np == NULL) {
        return -1;
    } else {
        if (previous) {
            previous->next = np->next;
        } else {
            hashtab[hash_string(key)] = np->next;
        }
        if (np->value && table->destructor) {
            table->destructor(np->value);
        }
        platform_free(np->key);
        platform_free(np);
    }
    return 0;
}

size_t hashtable_count(HashTable *table)
{
    size_t count = 0;
    for (int i = 0; i < HASHSIZE; ++i) {
        if (table->entries[i] == NULL) {
            continue;
        }
        for (HashTableEntry *entry = table->entries[i]; entry != NULL; entry = entry->next) {
            ++count;
        }
    }
    return count;
}

void hashtable_entry_destroy(HashTable *table, HashTableEntry *entry)
{
    if (entry->next) {
        hashtable_entry_destroy(table, entry->next);
        entry->next = NULL;
    }
    if (entry->value && table->destructor) {
        table->destructor(entry->value);
    }
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
        hashtable_entry_destroy(table, table->entries[i]);
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
    HashTable *table = (HashTable *)object;
    StringBuilder *sb = sb_create();

    for (size_t i = 0; i < HASHSIZE; ++i) {
        if (table->entries[i] == NULL) {
            continue;
        }
        sb_append_format(sb, " (%d)\n", i);
        for (HashTableEntry *entry = table->entries[i]; entry != NULL; entry = entry->next) {
            if (entry->value) {
                char *description = describe(entry->value);
                sb_append_format(sb, "%s: %s", entry->key, description);
                platform_free(description);
            } else {
                sb_append_format(sb, "%s: (NULL)", entry->key);
            }
            sb_append_line_break(sb);
            if (entry->next) {
                sb_append_string(sb, " -> ");
            } else {
            }
        }
    }
    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

HashTable *hashtable_create_with_destructor(void (*destructor)(void *))
{
    HashTable *table = platform_calloc(1, sizeof(HashTable));
    table->w_type = &HashTableType;
    
    void *entries = platform_calloc(HASHSIZE, sizeof(HashTableEntry *));
    table->entries = entries;
    table->destructor = destructor;
    
    return table;
}

HashTable *hashtable_create_with_weak_references()
{
    return hashtable_create_with_destructor(NULL);
}

HashTable *hashtable_create()
{
    return hashtable_create_with_destructor(&destroy);
}
