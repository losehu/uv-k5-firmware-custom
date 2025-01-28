#include <math.h>
#include "sgp.h"
#include "util.h"
#include "vec.h"

static const double SDP_THRESH_FREQ = 1440.0 / 225.0;
static const double XKMPER = 6378.135;
static const double ONE_HALF = 1.0 / 2.0;
static const double THREE_HALVES = 3.0 / 2.0;
static const double TWO_THIRDS = 2.0 / 3.0;

static const double a_E = 1.0;
static const double Q_0 = 1.88027916E-9;

static const double k_e = 0.0743669161;
static const double k_2 = 5.413080E-4;
static const double k_4 = 0.62098875E-6;
static const double S = 1.0122292801892716;
static const double A_3_COMMA_0 = 2.53881E-06;

sgp_result sgp4(tle_data *data, double minutes_since_epoch) {
    if (data->rev_per_day < SDP_THRESH_FREQ) {
        sgp_result fail = { {0}, {0}, {0}, {0} };
        return fail;
    }

    double i_0 = to_radians(data->inclination);
    double OMEGA_0 = to_radians(data->r_node_ascension);
    double M_0 = to_radians(data->mean_anomaly);
    double omega_0 = to_radians(data->perigee_arg);
    double xno = data->rev_per_day * 2 * M_PI / 1440.0;
    double delta_multiplier = ((3 * square(cos(i_0)) - 1) /
                               pow(1 - square(data->eccentricity), THREE_HALVES));

    double a_1 = pow(k_e / xno, TWO_THIRDS);
    double delta_1 = THREE_HALVES * (k_2 / square(a_1)) * delta_multiplier;
    double a_0 = a_1 * (1 - ((1.0 / 3.0) * delta_1) - square(delta_1) - ((134.0 / 81.0) * cube(delta_1)));
    double delta_0 = THREE_HALVES * (k_2 / square(a_0)) * delta_multiplier;
    double d_d_n_0 = xno / (1 + delta_0); /* original mean motion */
    double d_d_a_0 = a_0 / (1 - delta_0); /* semimajor axis */

    unsigned int simple = 0;
    if ((d_d_a_0 * (1.0 - data->eccentricity) / a_E) < (220.0 / XKMPER + a_E)) {
        simple = 1;
    }

    double perigee = (d_d_a_0 * (1 - data->eccentricity) - a_E) * XKMPER;
    double s = S;
    double q_0_minus_s_pow_4 = Q_0;
    if (perigee >= 98 && perigee <= 156) {
        s = (d_d_a_0 * (1 - data->eccentricity)) - S + a_E; /* s_star */
        q_0_minus_s_pow_4 = pow(Q_0 - s, 4.0);
    } else if (perigee < 98) {
        s = 20.0 / XKMPER + a_E; /* s_star */
        q_0_minus_s_pow_4 = pow(Q_0 - s, 4.0);
    }

    double theta = cos(i_0);
    double xi = 1 / (d_d_a_0 - s);
    double beta_0 = sqrt(1 - square(data->eccentricity));
    double eta = d_d_a_0 * data->eccentricity * xi;
    double C_2 =
            q_0_minus_s_pow_4 * pow(xi, 4.0) * d_d_n_0 * pow(1 - square(eta), -7.0 / 2.0) *
            (d_d_a_0 * (1 + THREE_HALVES * square(eta) + 4 * data->eccentricity * eta + data->eccentricity * cube(eta)) +
             (THREE_HALVES * ((k_2 * xi) / (1 - square(eta)))) *
             (-ONE_HALF + THREE_HALVES * square(theta)) *
             (8 + 24 * square(eta) + 3 * pow(eta, 4.0)));
    double C_1 = data->drag * C_2;
    double C_3 = (q_0_minus_s_pow_4 * pow(xi, 5.0) * A_3_COMMA_0 * d_d_n_0 * a_E * sin(i_0)) /
                 (k_2 / data->eccentricity);
    double C_4 = 2 * d_d_n_0 * q_0_minus_s_pow_4 * pow(xi, 4.0) * d_d_a_0 * square(beta_0) * pow(1 - square(eta), -7.0 / 2.0) *
                 ((2 * eta * (1 + data->eccentricity * eta) + ONE_HALF * data->eccentricity + ONE_HALF * cube(eta)) -
                  ((2 * k_2 * xi) / (d_d_a_0 * (1 - square(eta)))) *
                  (
                          3 * (1 - 3 * square(theta)) *
                          (1 + THREE_HALVES * square(eta) - 2 * data->eccentricity * eta - ONE_HALF * data->eccentricity * cube(eta)) +
                          (3.0 / 4.0) * (1 - square(theta)) * (2 * square(eta) - data->eccentricity * eta - data->eccentricity * cube(eta)) * cos(2 * omega_0))
                  );
    double C_5 = 2 * q_0_minus_s_pow_4 * pow(xi, 4.0) * d_d_a_0 * square(beta_0) *
                 pow(1 - square(eta), -7.0 / 2.0) *
                 (1 + (11.0 / 4.0) * eta * (eta + data->eccentricity) + data->eccentricity * cube(eta));

    double M_DF = M_0 +
                  (1 + ((3 * k_2 * (-1 + 3 * square(theta))) / (2 * square(d_d_a_0) * cube(beta_0))) +
                   ((3 * square(k_2) * (13 - 78 * square(theta) + 137 * pow(theta, 4.0))) /
                    (16 * pow(d_d_a_0, 4.0) * pow(beta_0, 7.0)))) *
                  d_d_n_0 * minutes_since_epoch;
    double omega_DF = omega_0 +
                      (-((3 * k_2 * (1 - 5 * square(theta))) / (2 * square(d_d_a_0) * pow(beta_0, 4.0))) +
                       ((3 * square(k_2) * (7 - 114 * square(theta) + 395 * pow(theta, 4.0))) /
                        (16 * pow(d_d_a_0, 4.0) * pow(beta_0, 8.0))) +
                       ((5 * k_4 * (3 - 36 * square(theta) + 49 * pow(theta, 4.0))) /
                        (4.0 * pow(d_d_a_0, 4.0) * pow(beta_0, 8.0)))) *
                      d_d_n_0 * minutes_since_epoch;
    double OMEGA_DF = OMEGA_0 +
                      ((-(3 * k_2 * theta) / (square(d_d_a_0) * pow(beta_0, 4.0))) +
                       ((3 * square(k_2) * (4 * theta - 19 * cube(theta))) /
                        (2 * pow(d_d_a_0, 4.0) * pow(beta_0, 8.0))) +
                       ((5 * k_4 * theta * (3 - 7 * square(theta))) / (2 * pow(d_d_a_0, 4.0) * pow(beta_0, 8.0)))) *
                      d_d_n_0 * minutes_since_epoch;
    double delta_omega = data->drag * C_3 * cos(omega_0) * minutes_since_epoch;
    double delta_M = -TWO_THIRDS * q_0_minus_s_pow_4 * data->drag * pow(xi, 4.0) *
                     (a_E / (data->eccentricity * eta)) *
                     (pow(1 + eta * cos(M_DF), 3.0) - pow(1 + eta * cos(M_0), 3.0));
    double M_p = M_DF;
    double omega = omega_DF;
    double OMEGA = OMEGA_DF -
                   (21.0 / 2.0) * ((d_d_n_0 * k_2 * theta) / (square(d_d_a_0) * square(beta_0))) *
                   C_1 * square(minutes_since_epoch);

    double e;
    double a;
    double L;
    if (simple == 0) {
        M_p += delta_omega + delta_M;
        omega -= delta_omega - delta_M;
        e = data->eccentricity - data->drag * C_4 * minutes_since_epoch -
                   data->drag * C_5 * (sin(M_p) - sin(M_0));

        double D_2 = 4 * d_d_a_0 * xi * square(C_1);
        double D_3 = (4.0 / 3.0) * d_d_a_0 * square(xi) * (17 * d_d_a_0 + s) * cube(C_1);
        double D_4 = TWO_THIRDS * d_d_a_0 * cube(xi) * (221 * d_d_a_0 + 31 * s) * pow(C_1, 4.0);
        a = d_d_a_0 *
                   square(1 - C_1 * minutes_since_epoch -
                          D_2 * square(minutes_since_epoch) -
                          D_3 * cube(minutes_since_epoch) -
                          D_4 * pow(minutes_since_epoch, 4.0));
        L = M_p + omega + OMEGA + d_d_n_0 * (THREE_HALVES * C_1 * square(minutes_since_epoch) +
                                                    (D_2 + 2 * square(C_1)) * cube(minutes_since_epoch) +
                                                    (1.0 / 4.0) * (3 * D_3 + 12 * C_1 * D_2 + 10 * cube(C_1) * pow(minutes_since_epoch, 4.0)) +
                                                    ((1.0 / 5.0) *
                                                     (3 * D_4 + 12 * C_1 * D_3 + 6 * square(D_2) + 30 * square(C_1) * D_2 +
                                                      15 * pow(C_1, 4.0) * pow(minutes_since_epoch, 5.0))));
    } else {
        e = data->eccentricity - data->drag * C_4 * minutes_since_epoch;
        a = d_d_a_0 * square(1 - C_1 * minutes_since_epoch);
        L = M_p + omega + OMEGA + d_d_n_0 * (THREE_HALVES * C_1 * square(minutes_since_epoch));
    }
    double beta = sqrt(1 - square(e));
    double n = k_e / sqrt(cube(a));

    double a_xN = e * cos(omega);
    double L_L = ((A_3_COMMA_0 * sin(i_0)) / (8 * k_2 * a * square(beta))) *
                 (e * cos(omega)) * ((3 + 5 * theta) / (1 + theta));
    double a_yNL = (A_3_COMMA_0 * sin(i_0)) / (4 * k_2 * a * square(beta));
    double L_T = L + L_L;
    double a_yN = e * sin(omega) + a_yNL;

    double AXN_SIN;
    double AYN_COS;
    double AXN_COS;
    double AYN_SIN;
    double U = fmod(L_T - OMEGA, 2 * M_PI);
    double E_w_i = U;
    for (int i = 1; i <= 10; ++i) {
        double sinpw = sin(E_w_i);
        double cospw = cos(E_w_i);

        AXN_SIN = a_xN * sinpw;
        AYN_COS = a_yN * cospw;
        AXN_COS = a_xN * cospw;
        AYN_SIN = a_yN * sinpw;

        double dE_w_i = (U - AYN_COS + AXN_SIN - E_w_i) /
                        (-AYN_SIN - AXN_COS + 1) + E_w_i;
        if (fabs(dE_w_i - E_w_i) <= 1.0E-6) {
            break;
        }

        E_w_i = dE_w_i;
    }

    double e_cos_E = AXN_COS + AYN_SIN;
    double e_sin_E = AXN_SIN - AYN_COS;
    double e_L = sqrt(square(a_xN) + square(a_yN));
    double p_L = a * (1 - square(e_L));
    double r = a * (1 - e_cos_E);
    double r_dot = k_e * (sqrt(a) / r) * e_sin_E;
    double r_f_dot = k_e * (sqrt(p_L) / r);
    double cos_u = (a / r) * (cos(E_w_i) - a_xN + ((a_yN * e_sin_E) / (1 + sqrt(1 - square(e_L)))));
    double sin_u = (a / r) * (sin(E_w_i) - a_yN - ((a_xN * e_sin_E) / (1 + sqrt(1 - square(e_L)))));
    double u = atan2(sin_u, cos_u);
    double dr = (k_2 / (2 * p_L)) * (1 - square(theta)) * cos(2 * u);
    double du = (-k_2 / (4 * square(p_L))) * (7 * square(theta) - 1) * sin(2 * u);
    double dOMEGA = ((3 * k_2 * theta) / (2 * square(p_L))) * sin(2 * u);
    double di = ((3 * k_2 * theta) / (2 * square(p_L))) * sin(i_0) * cos(2 * u);
    double dr_dot = (-(k_2 * n) / p_L) * (1 - square(theta)) * sin(2 * u);
    double dr_f_dot = ((k_2 * n) / p_L) * ((1 - square(theta)) * cos(2 * u) - THREE_HALVES * (1 - 3 * square(theta)));
    double r_k = r * (1 - THREE_HALVES * k_2 * (sqrt(1 - square(e_L)) / square(p_L)) * (3 * square(theta) - 1)) + dr;
    double u_k = u + du;
    double OMEGA_k = OMEGA + dOMEGA;
    double i_k = i_0 + di;
    double r_dot_k = r_dot + dr_dot;
    double r_f_dot_k = r_f_dot + dr_f_dot;

    vec M = {-sin(OMEGA_k) * cos(i_k), cos(OMEGA_k) * cos(i_k), sin(i_k)};
    vec N = {cos(OMEGA_k), sin(OMEGA_k), 0};

    vec result_U = vec_add(vec_mul(sin(u_k), M), vec_mul(cos(u_k), N));
    vec result_V = vec_add(vec_mul(cos(u_k), M), vec_mul(-sin(u_k), N));

    vec result_r = vec_mul(XKMPER, vec_mul(r_k, result_U));
    vec result_r_dot = vec_mul(XKMPER / 60, vec_add(vec_mul(r_dot_k, result_U), vec_mul(r_f_dot_k, result_V)));

    sgp_result result = {result_U, result_V, result_r, result_r_dot};
    return result;
}