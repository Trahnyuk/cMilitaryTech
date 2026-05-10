#include "AmmoParameters.h"
#include "MissionInput.h"
#include "FireSolution.h"

namespace cMiltech {

    class BallisticCalculator {
    public:
      
        static FireSolution compute(const MissionInput&   input,
                                    const AmmoParameters& ammo);

    private:
        // Step 1 — solve cubic equation (Cardano) for time of flight
        static float solveTimeOfFlight(float V0, float Z0,
                                       float m,  float d, float l);

        // Step 2 — horizontal distance (power series to t^5)
        static float computeHorizontalDistance(float t,  float V0,
                                               float m,  float d, float l);

        // Step 3 — fire point and optional maneuver point
        static FireSolution buildSolution(const MissionInput& input,
                                          float hDistance,
                                          float t);

        static constexpr float G = 9.81f;
    };

} 
