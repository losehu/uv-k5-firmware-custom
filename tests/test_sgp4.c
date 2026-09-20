#include <math.h>
#include <stdio.h>

#include "tle/sgp.h"

static int check_vector(const char *name, vec actual, vec expected, double tolerance)
{
    const double max_error = fmax(fabs(actual.x - expected.x),
                                  fmax(fabs(actual.y - expected.y),
                                       fabs(actual.z - expected.z)));
    if (!isfinite(actual.x) || !isfinite(actual.y) || !isfinite(actual.z) ||
        max_error > tolerance) {
        fprintf(stderr,
                "%s failed: got %.12f %.12f %.12f, max error %.12g\n",
                name, actual.x, actual.y, actual.z, max_error);
        return 1;
    }
    return 0;
}

int main(void)
{
    /* CelesTrak SGP4 verification case 22312: low perigee (73.54 km). */
    tle_data low_perigee = {
        .drag = 0.00049949,
        .inclination = 62.1486,
        .r_node_ascension = 77.4698,
        .eccentricity = 0.0308723,
        .perigee_arg = 267.9229,
        .mean_anomaly = 88.7392,
        .rev_per_day = 15.95744531,
    };
    const sgp_result low_result = sgp4(&low_perigee, 1.0);
    const vec low_position = {
        1230.546156441918,
        6553.770748054392,
        417.855528029076,
    };

    /* CelesTrak case 06251: non-simple propagation exercises C3 drag. */
    tle_data moderate_drag = {
        .drag = 0.00012808,
        .inclination = 58.0579,
        .r_node_ascension = 54.0425,
        .eccentricity = 0.0030035,
        .perigee_arg = 139.1568,
        .mean_anomaly = 221.1854,
        .rev_per_day = 15.56387291,
    };
    const sgp_result drag_result = sgp4(&moderate_drag, 2880.0);
    const vec drag_position = {
        1159.457330440577,
        5056.670744799598,
        4353.366641428027,
    };

    int failed = 0;
    failed |= check_vector("low-perigee position", low_result.r, low_position, 1.0e-4);
    failed |= check_vector("moderate-drag position", drag_result.r, drag_position, 1.0e-4);
    if (failed == 0)
        puts("SGP4 regression tests passed");
    return failed;
}
