#include "tle.h"
#include "vec.h"
#include <string.h>

#ifndef FATE_SGP_H
#define FATE_SGP_H

typedef struct {
    vec u; /* orientation */
    vec v;

    vec r; /* position */
    vec r_dot; /* velocity */
} sgp_result;

sgp_result sgp4(tle_data *data, double minutes_since_epoch);

#endif /* FATE_SGP_H */
