#ifndef hash_table_private_h
#define hash_table_private_h

struct HashTable {
    BASE_OBJECT;
    HashTableEntry **entries;
};

#endif /* hash_table_private_h */
