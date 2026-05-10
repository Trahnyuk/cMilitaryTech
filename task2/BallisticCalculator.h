#ifndef BALLISTICCALCULATOR_H
#define BALLISTICCALCULATOR_H

#include "AmmoParameters.h"
#include "BallisticResult.h"

namespace cMiltech {

 
    class BallisticCalculator {
    public:
        // Solve the cubic + power series for the given drop conditions.
        static BallisticResult compute(float V0, float Z0,
                                       const AmmoParameters& ammo);

    private:
        // Step 1 — cubic equation (Cardano) for time of flight
        static float solveTimeOfFlight(float V0, float Z0,
                                       float m, float d, float l);

        // Step 2 — horizontal distance (power series to t^5)
        static float computeHorizontalDistance(float t, float V0,
                                               float m, float d, float l);

        static constexpr float G = 9.81f;
    };

}

#endif // BALLISTICCALCULATOR_H
