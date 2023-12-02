#ifndef random_h
#define random_h

#include "types.h"

typedef struct Random Random;

Random *random_create(uint64_t seed_a, uint64_t seed_b);
uint64_t random_next_uint64(Random *);
bool random_next_bool(Random *);
Float random_next_float_limit(Random *, Float limit);
Float random_next_float(Random *);
int random_next_int(Random *);
int random_next_int_limit(Random *, int limit);

void random_shake(Random *, uint64_t value);
void random_shake_using_current_time(Random *);

#endif /* random_h */
