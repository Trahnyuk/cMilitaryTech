#define _USE_MATH_DEFINES
#include "BallisticCalculator.h"

#include <cmath>
#include <stdexcept>

using std::sqrt;
using std::pow;
using std::acos;
using std::cos;
using std::fabs;
using std::runtime_error;

namespace cMiltech {

    BallisticResult BallisticCalculator::compute(float V0, float Z0,
                                                  const AmmoParameters& ammo) {
        const float m = ammo.getMass();
        const float d = ammo.getDrag();
        const float l = ammo.getLift();

        const float t = solveTimeOfFlight(V0, Z0, m, d, l);
        const float h = computeHorizontalDistance(t, V0, m, d, l);
        return BallisticResult{ t, h };
    }


    // Step 1: Cardano trigonometric solution for a*t^3 + b*t^2 + c = 0

    float BallisticCalculator::solveTimeOfFlight(float V0, float Z0,
                                                  float m, float d, float l) {
        const float a = d * G * m - 2.0f * d * d * l * V0;
        const float b = -3.0f * G * m * m + 3.0f * d * l * m * V0;
        const float c = 6.0f * m * m * Z0;

        const float p = -(b * b) / (3.0f * a * a);
        const float q = (2.0f * b * b * b) / (27.0f * a * a * a) + c / a;

        if (p >= 0.0f) {
            throw runtime_error("No real solution for time of flight (p >= 0).");
        }

        const float acosArg = (3.0f * q / (2.0f * p)) * sqrt(-3.0f / p);
        if (fabs(acosArg) > 1.0f) {
            throw runtime_error("No real solution for time of flight (arccos arg out of range).");
        }

        const float phi = acos(acosArg);
        const float t   = 2.0f * sqrt(-p / 3.0f)
                        * cos((phi + 4.0f * static_cast<float>(M_PI)) / 3.0f)
                        - b / (3.0f * a);

        if (t <= 0.0f) {
            throw runtime_error("Computed time of flight is non-positive.");
        }
        return t;
    }


    // Step 2: Horizontal distance — power series to t^5

    float BallisticCalculator::computeHorizontalDistance(float t, float V0,
                                                          float m, float d, float l) {
        const float t2 = pow(t, 2.0f);
        const float t3 = pow(t, 3.0f);
        const float t4 = pow(t, 4.0f);
        const float t5 = pow(t, 5.0f);
        const float d2 = pow(d, 2.0f);
        const float d3 = pow(d, 3.0f);
        const float d4 = pow(d, 4.0f);
        const float l2 = pow(l, 2.0f);
        const float l3 = pow(l, 3.0f);
        const float l4 = pow(l, 4.0f);
        const float m2 = pow(m, 2.0f);
        const float m3 = pow(m, 3.0f);
        const float m4 = pow(m, 4.0f);

        const float term1 = t * V0;
        const float term2 = -(t2 * d * V0) / (2.0f * m);

        const float term3 = (t3 * (6.0f * d * G * l * m
                           - 6.0f * d2 * (l2 - 1.0f) * V0))
                          / (36.0f * m2);

        const float denom4 = 36.0f * pow(1.0f + l2, 2.0f) * m3;
        const float term4  = (t4 * (-6.0f * d2 * G * l * (1.0f + l2 + l4) * m
                            + 3.0f * d3 * l2 * (1.0f + l2) * V0
                            + 6.0f * d3 * l4 * (1.0f + l2) * V0))
                           / denom4;

        const float denom5 = 36.0f * (1.0f + l2) * m4;
        const float term5  = (t5 * (3.0f * d3 * G * l3 * m
                            - 3.0f * d4 * l2 * (1.0f + l2) * V0))
                           / denom5;

        return term1 + term2 + term3 + term4 + term5;
    }

}
