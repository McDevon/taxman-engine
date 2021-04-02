#ifndef hash_table_private_h
#define hash_table_private_h

struct HashTable {
    BASE_OBJECT;
    HashTableEntry **entries;
};

struct HashTableEntry {
    struct HashTableEntry *next;
    char *key;
    void *value;
};

#endif /* hash_table_private_h */
