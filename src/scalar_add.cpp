#include "scalar_add.h"

void add_scalar(int size, int *a, int *b) {
    for (int i = 0; i < size; i++) {
        a[i] += b[i];
    }
}