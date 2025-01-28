#include <stdio.h>
#include "vec.h"

vec vec_mul(double scalar, vec vector) {
    vec result = {vector.x * scalar, vector.y * scalar, vector.z * scalar};
    return result;
}

vec vec_add(vec vector1, vec vector2) {
    vec result = {vector1.x + vector2.x, vector1.y + vector2.y, vector1.z + vector2.z};
    return result;
}

void vec_print(FILE *stream, vec vector) {
    fprintf(stream, "{ %f, %f, %f }\n", vector.x, vector.y, vector.z);
}
