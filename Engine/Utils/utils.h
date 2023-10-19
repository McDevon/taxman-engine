#ifndef utils_h
#define utils_h

int mod(int a, int b);

#define max(a,b) \
({ __typeof__ (a) __a = (a); \
    __typeof__ (b) __b = (b); \
  __a > __b ? __a : __b; })

#define min(a,b) \
({ __typeof__ (a) __a = (a); \
    __typeof__ (b) __b = (b); \
  __a < __b ? __a : __b; })

#define floor_div(a,b) \
({ __typeof__ (a) __a = (a); \
    __typeof__ (b) __b = (b); \
  (__a >= 0 ? __a / __b : ((__a + 1) / __b) - 1); })

#define f_lerp(a, b, f) \
({ __typeof__ (f) __f = (f); \
  (a) * (1.f - __f) + (b) * __f; })

#define for_each_begin(type, item, list) { type item = NULL; \
const size_t for_each_count = list_count(list); \
for (size_t for_each_index = 0; for_each_index < for_each_count; ++for_each_index) { \
item = (type)list_get(list, for_each_index);

#define for_each_end }}

#define is_power_of_two(value) \
({ __typeof__ (value) __value = (value); \
  ((__value != 0) && ((__value & (__value - 1)) == 0)); })

#endif /* utils_h */
