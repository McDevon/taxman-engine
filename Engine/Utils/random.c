#include "random.h"
#include <stdint.h>
#include "base_object.h"
#include "string_builder.h"
#include "platform_adapter.h"
#include <limits.h>

struct Random {
    BASE_OBJECT;
    uint64_t a, b;
};

void random_destroy(void *value)
{
}

char *random_describe(void *value)
{
    Random *rand = (Random *)value;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "state: ");
    sb_append_uint64(sb, rand->a);
    sb_append_string(sb, ", ");
    sb_append_uint64(sb, rand->b);

    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;}

static BaseType RandomType = { "Random", &random_destroy, &random_describe };

Random *random_create(uint64_t seed_a, uint64_t seed_b)
{
    Random *random = platform_calloc(1, sizeof(Random));
    random->a = seed_a;
    random->b = seed_b;

    random->w_type = &RandomType;
    
    return random;
}

uint64_t random_next_uint64(Random *state)
{
    // XorShift128+ from https://en.wikipedia.org/wiki/Xorshift#xorshift+
    uint64_t t = state->a;
    uint64_t const s = state->b;
    state->a = s;
    t ^= t << 23;
    t ^= t >> 17;
    t ^= s ^ (s >> 26);
    state->b = t;
    return t + s;
}

bool random_next_bool(Random *state)
{
    return random_next_uint64(state) & (1LLU << 20) ? true : false;
}

Float random_next_float_limit(Random *state, Float limit)
{
    return (Float)random_next_uint64(state) / (Float)UINT64_MAX * limit;
}

Float random_next_float(Random *state)
{
    return (Float)random_next_uint64(state) / (Float)UINT64_MAX;
}

int random_next_int(Random *state)
{
    return (int)(random_next_uint64(state) & INT_MAX);
}

int random_next_int_limit(Random *state, int limit)
{
    return (int)((random_next_uint64(state) & INT_MAX) % limit);
}
