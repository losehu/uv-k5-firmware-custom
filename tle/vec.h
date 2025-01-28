#ifndef FATE_VEC_H
#define FATE_VEC_H
#include <string.h>

#include <stdio.h>

typedef struct {
    double x;
    double y;
    double z;
} vec;

/**
 * Performs a scalar multiplication on the given vector.
 *
 * @return a new vector containing the multiplied values
 */
vec vec_mul(double, vec);

/**
 * Adds the values of two vectors together.
 *
 * @return a new vector containing the added values
 */
vec vec_add(vec, vec);

/**
 * Prints the given vector to the given stream.
 */
void vec_print(FILE *stream, vec);

#endif /* FATE_VEC_H */
