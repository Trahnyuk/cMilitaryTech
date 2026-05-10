#ifndef BALLISTICRESULT_H
#define BALLISTICRESULT_H

namespace cMiltech {

    // Pure ballistic outcome of a drop: time of flight and horizontal range.
    // The fire-point geometry is now decided per-step by the simulator.
    struct BallisticResult {
        float timeOfFlight;
        float horizontalDistance;
    };

}

#endif // BALLISTICRESULT_H
