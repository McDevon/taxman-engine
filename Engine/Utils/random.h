#ifndef random_h
#define random_h

#include "types.h"
#include "number.h"

typedef struct Random Random;

Random *random_create(uint64_t seed_a, uint64_t seed_b);
uint64_t random_next_uint64(Random *);
Bool random_next_bool(Random *);
Number random_next_number(Random *);

#endif /* random_h */
