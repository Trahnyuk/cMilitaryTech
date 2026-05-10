#ifndef DRONE_H
#define DRONE_H

#include "DroneState.h"
#include "Geometry.h"

namespace cMiltech {

    // Drone state machine matching the reference implementation pattern:
    //   - Each state computes deltaPath (displacement magnitude)
    //   - Position updated once after the switch
    //   - Trapezoidal integration for accel/decel phases
    //   - Direction snap within turnThreshold
    //   - TURNING re-evaluates toward current desiredDir each tick
    class Drone {
    public:
        Drone(Point2D pos, float zd, float dir,
              float attackSpeed, float accelerationPath,
              float angularSpeed, float turnThreshold,
              float hitRadius);

        // Advance one tick. Returns true if the final-action condition is met.
        // allowDrop=false is used for intermediate waypoint legs so the drone
        // can move safely without triggering the final event.
        bool step(float dt, Point2D movementGoal, int targetIndex, bool allowDrop);

        // Backward-compatible wrapper for existing call sites.
        bool step(float dt, Point2D movementGoal, int targetIndex) {
            return step(dt, movementGoal, targetIndex, true);
        }

        float timeToStop() const noexcept;
        float estimateTimeToFirePoint(Point2D firePoint) const;

        Point2D    position()    const noexcept { return m_pos; }
        float      zd()          const noexcept { return m_zd; }
        float      direction()   const noexcept { return m_dir; }
        float      speed()       const noexcept { return m_speed; }
        DroneState state()       const noexcept { return m_state; }
        int        targetIndex() const noexcept { return m_currentTarget; }

    private:
        Point2D    m_pos;
        float      m_zd;
        float      m_dir;
        float      m_speed{0.0f};

        float      m_attackSpeed;
        float      m_accelerationPath;
        float      m_acceleration;
        float      m_angularSpeed;
        float      m_turnThreshold;
        float      m_hitRadius;

        DroneState m_state{DroneState::Stopped};
        int        m_currentTarget{-1};

        static float normalizeAngle(float a);
        static float angleDiff(float from, float to);
        float        angleToward(Point2D fp) const;
    };

}

#endif // DRONE_H
