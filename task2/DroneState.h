#ifndef DRONESTATE_H
#define DRONESTATE_H

namespace cMiltech {

    // Numeric values must match the simulation.txt output spec (HW2, §5).
    enum class DroneState : int {
        Stopped      = 0,
        Accelerating = 1,
        Decelerating = 2,
        Turning      = 3,
        Moving       = 4
    };

}

#endif // DRONESTATE_H
