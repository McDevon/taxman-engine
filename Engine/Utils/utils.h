#ifndef utils_h
#define utils_h

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

#endif /* utils_h */
