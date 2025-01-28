#ifndef FATE_UTIL_H
#define FATE_UTIL_H
#define M_PI 3.14159265358979323846
#include <stdlib.h>
#include <math.h>
#include <assert.h>

/**
 * Multiplies the given number by itself.
 *
 * CONSTRAINT: x <= 1.34E154
 */
static inline double square(double x) {
    return x * x;
}

/**
 * Multiplies the given number times itself 3 times.
 *
 * CONSTRAINT:  x <= 5.64E102
 */
static inline double cube(double x) {
    return x * x * x;
}

/**
 * Obtains the fractional portion of a double value.
 *
 * @param x the value which to obtain the fraction
 * @return the fractional portion
 */
static inline double frac(double x) {
    return x - (long) x;
}

/**
 * Converts the given number to radians.
 *
 * CONSTRAINT: x is in degrees
 * CONSTRAINT: x <= 1.14E306
 */
static inline double to_radians(double x) {
    static const double MULTIPLIER = M_PI / 180;

    return x * MULTIPLIER;
}

/**
 * Converts the given number to degrees.
 *
 * CONSTRAINT: x is in radians
 * CONSTRAINT: x <= 9.98E305
 */
static inline double to_degrees(double x) {
    static const double MULTIPLIER = 180 / M_PI;

    return x * MULTIPLIER;
}

/**
 * Takes the substring of the given string, starting from
 * start and ending len characters later.
 *
 * CONSTRAINT: start >= 0
 * CONSTRAINT: start + len <= strlen(str)
 *
 * @param str the string to obtain the substring
 * @param start the beginning index, inclusive
 * @param len the number of characters to copy
 * @return the new substring, or NULL on failure
 */
char substr(const char *str, int start, int len, char *buf);

/**
 * Converts a given string into a long value.
 *
 * @param str the string to convert
 * @param value the pointer at which to store the result
 * @return 0 on failure
 */
int strl(const char *str, long *value);

/**
 * Converts a given string into a double value.
 *
 * @param str the string to convert
 * @param value the pointer at which to store the result
 * @return 0 on failure
 */
int strd(const char *str, double *value);

#endif /* FATE_UTIL_H */
