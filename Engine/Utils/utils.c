#include <stdio.h>

int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}
