#include "Drone.h"

#include <algorithm>
#include <cmath>

namespace cMiltech {

    namespace {
        constexpr float PI     = 3.14159265358979323846f;
        constexpr float TWO_PI = 2.0f * PI;
    }

    Drone::Drone(Point2D pos, float zd, float dir,
                 float attackSpeed, float accelerationPath,
                 float angularSpeed, float turnThreshold,
                 float hitRadius)
        : m_pos(pos)
        , m_zd(zd)
        , m_dir(normalizeAngle(dir))
        , m_attackSpeed(attackSpeed)
        , m_accelerationPath(accelerationPath)
        , m_acceleration((attackSpeed * attackSpeed) / (2.0f * accelerationPath))
        , m_angularSpeed(angularSpeed)
        , m_turnThreshold(turnThreshold)
        , m_hitRadius(hitRadius)
    {}

    float Drone::normalizeAngle(float a) {
        while (a >  PI) a -= TWO_PI;
        while (a < -PI) a += TWO_PI;
        return a;
    }

    float Drone::angleDiff(float from, float to) {
        return normalizeAngle(to - from);
    }

    float Drone::angleToward(Point2D fp) const {
        return std::atan2(fp.y - m_pos.y, fp.x - m_pos.x);
    }

    bool Drone::step(float dt, Point2D movementGoal, int targetIndex, bool allowDrop) {
        m_currentTarget = targetIndex;

        const float desiredDir = angleToward(movementGoal);
        const float deltaAngle = angleDiff(m_dir, desiredDir);
        const float adelta     = std::fabs(deltaAngle);

        float deltaPath = 0.0f;

        switch (m_state) {

        case DroneState::Stopped: {
            if (adelta > m_turnThreshold) {
                m_state = DroneState::Turning;
            } else {
                m_dir   = desiredDir;
                m_state = DroneState::Accelerating;
            }
            break;
        }

        case DroneState::Accelerating: {
            if (adelta > m_turnThreshold && m_speed > 0.01f) {
                m_state = DroneState::Decelerating;

                const float prevSpeed = m_speed;
                m_speed -= m_acceleration * dt;

                if (m_speed <= 0.0f) {
                    m_speed = 0.0f;
                    m_state = DroneState::Stopped;
                }

                deltaPath = (prevSpeed + m_speed) / 2.0f * dt;
            } else {
                const float prevSpeed = m_speed;
                m_speed += m_acceleration * dt;

                if (m_speed >= m_attackSpeed) {
                    m_speed = m_attackSpeed;
                    m_state = DroneState::Moving;
                }

                const float aTang = (m_speed - prevSpeed) / dt;
                const float aBudget = 0.9f * std::sqrt(
                    std::max(0.0f, m_acceleration * m_acceleration - aTang * aTang)
                );

                const float maxStep = (m_speed > 0.01f)
                    ? std::min(m_angularSpeed * dt, aBudget * dt / m_speed)
                    : m_angularSpeed * dt;

                if (adelta <= maxStep) {
                    m_dir = desiredDir;
                } else {
                    const float sign = (deltaAngle > 0.0f) ? 1.0f : -1.0f;
                    m_dir = normalizeAngle(m_dir + sign * maxStep);
                }

                deltaPath = (prevSpeed + m_speed) / 2.0f * dt;
            }
            break;
        }

        case DroneState::Decelerating: {
            const float prevSpeed = m_speed;
            m_speed -= m_acceleration * dt;

            if (m_speed <= 0.0f) {
                m_speed = 0.0f;
                m_state = DroneState::Stopped;
            }

            deltaPath = (prevSpeed + m_speed) / 2.0f * dt;
            break;
        }

        case DroneState::Turning: {
            const float da = angleDiff(m_dir, desiredDir);

            if (std::fabs(da) <= m_angularSpeed * dt) {
                m_dir   = desiredDir;
                m_state = DroneState::Accelerating;
            } else {
                const float sign = (da > 0.0f) ? 1.0f : -1.0f;
                m_dir = normalizeAngle(m_dir + sign * m_angularSpeed * dt);
            }
            break;
        }

        case DroneState::Moving: {
            if (adelta > m_turnThreshold) {
                m_state = DroneState::Decelerating;

                const float prevSpeed = m_speed;
                m_speed -= m_acceleration * dt;

                if (m_speed <= 0.0f) {
                    m_speed = 0.0f;
                    m_state = DroneState::Stopped;
                }

                deltaPath = (prevSpeed + m_speed) / 2.0f * dt;
            } else {
                const float maxStep = std::min(
                    m_angularSpeed * dt,
                    0.9f * m_acceleration * dt / m_speed
                );

                if (adelta <= maxStep) {
                    m_dir = desiredDir;
                } else {
                    const float sign = (deltaAngle > 0.0f) ? 1.0f : -1.0f;
                    m_dir = normalizeAngle(m_dir + sign * maxStep);
                }

                deltaPath = m_speed * dt;

                // Safety fix: intermediate waypoint legs are allowed to move,
                // but they must not trigger the final event.
                //
                // Also detect whether this discrete simulation step crosses the
                // movement goal. The previous version checked only the pre-move
                // distance, so with dt=0.1 and speed=10 it could jump over a
                // small trigger radius and report the event late.
                const Point2D oldPos = m_pos;
                const Point2D newPos{
                    m_pos.x + std::cos(m_dir) * deltaPath,
                    m_pos.y + std::sin(m_dir) * deltaPath
                };

                const float sx = newPos.x - oldPos.x;
                const float sy = newPos.y - oldPos.y;
                const float segLen2 = sx * sx + sy * sy;

                float closestDist = std::hypot(
                    movementGoal.x - oldPos.x,
                    movementGoal.y - oldPos.y
                );

                if (segLen2 > 1e-6f) {
                    float t = ((movementGoal.x - oldPos.x) * sx +
                               (movementGoal.y - oldPos.y) * sy) / segLen2;

                    t = std::max(0.0f, std::min(1.0f, t));

                    const Point2D closest{
                        oldPos.x + sx * t,
                        oldPos.y + sy * t
                    };

                    closestDist = std::hypot(
                        movementGoal.x - closest.x,
                        movementGoal.y - closest.y
                    );
                }

                if (allowDrop && closestDist <= m_hitRadius * 0.25f) {
                    m_pos = newPos;
                    return true;
                }
            }
            break;
        }

        } // switch

        m_pos.x += std::cos(m_dir) * deltaPath;
        m_pos.y += std::sin(m_dir) * deltaPath;

        return false;
    }

    float Drone::timeToStop() const noexcept {
        switch (m_state) {
        case DroneState::Stopped:      return 0.0f;
        case DroneState::Accelerating: return m_speed       / m_acceleration;
        case DroneState::Moving:       return m_attackSpeed / m_acceleration;
        case DroneState::Decelerating: return m_speed       / m_acceleration;
        case DroneState::Turning:      return 0.0f;
        }
        return 0.0f;
    }

    float Drone::estimateTimeToFirePoint(Point2D fp) const {
        float totalTime = 0.0f;
        float curX = m_pos.x;
        float curY = m_pos.y;
        float curDir = m_dir;
        float curSpeed = m_speed;

        const float desiredDir = angleToward(fp);
        const float a = m_acceleration;

        const bool needStop = (std::fabs(angleDiff(curDir, desiredDir)) > m_turnThreshold);
        if (needStop) {
            const float pathToStop = (curSpeed * curSpeed) / (2.0f * a);
            curX += std::cos(curDir) * pathToStop;
            curY += std::sin(curDir) * pathToStop;

            totalTime += curSpeed / a;
            totalTime += std::fabs(angleDiff(curDir, desiredDir)) / m_angularSpeed;

            curSpeed = 0.0f;
            curDir = desiredDir;
        }

        float distToTarget = std::hypot(fp.x - curX, fp.y - curY);

        const float decelPath = std::min(
            (m_attackSpeed * m_attackSpeed) / (2.0f * a),
            distToTarget
        );
        distToTarget -= decelPath;
        totalTime += std::sqrt(2.0f * decelPath / a);

        const float accelDist = std::min(
            (m_attackSpeed * m_attackSpeed) / (2.0f * a)
            - (curSpeed * curSpeed) / (2.0f * a),
            distToTarget
        );
        distToTarget -= accelDist;
        totalTime += std::sqrt(2.0f * accelDist / a);

        if (distToTarget > 0.0f) {
            totalTime += distToTarget / m_attackSpeed;
        }

        return totalTime;
    }

}
