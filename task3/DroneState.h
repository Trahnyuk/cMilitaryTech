#ifndef DRONESTATE_H
#define DRONESTATE_H

namespace cMiltech {

    enum class DroneState : int {
        Stopped      = 0,
        Accelerating = 1,
        Decelerating = 2,
        Turning      = 3,
        Moving       = 4
    };

}

#endif
