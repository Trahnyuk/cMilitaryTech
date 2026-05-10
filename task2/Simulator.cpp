#define _USE_MATH_DEFINES
#include "Simulator.h"
#include "BallisticCalculator.h"

#include <cmath>
#include <iostream>
#include <limits>

namespace cMiltech {

    namespace {
        constexpr float PI     = static_cast<float>(M_PI);
        constexpr float TWO_PI = 2.0f * PI;

        struct DropGeometry {
            Point2D firePoint{};
            Point2D intermediatePoint{};
            bool    hasIntermediate{false};
        };

        DropGeometry computeDropGeometry(Point2D drone, Point2D target,
                                          float hDist, float accPath) {
            DropGeometry g;
            const float dx = target.x - drone.x;
            const float dy = target.y - drone.y;
            const float D  = std::sqrt(dx * dx + dy * dy);

            g.hasIntermediate = (hDist + accPath > D);

            float curX = drone.x;
            float curY = drone.y;
            float distToTarget = D;

            if (g.hasIntermediate) {
                if (D < 1e-6f) {
                    curX = target.x - (hDist + accPath);
                    curY = target.y;
                    distToTarget = hDist + accPath;
                } else {
                    curX = target.x - dx * (hDist + accPath) / D;
                    curY = target.y - dy * (hDist + accPath) / D;
                    distToTarget = std::sqrt(
                        std::pow(target.x - curX, 2.0f) +
                        std::pow(target.y - curY, 2.0f)
                    );
                }
                g.intermediatePoint = { curX, curY };
            }

            if (distToTarget > 1e-6f) {
                const float ratio = (distToTarget - hDist) / distToTarget;
                g.firePoint = {
                    curX + (target.x - curX) * ratio,
                    curY + (target.y - curY) * ratio
                };
            } else {
                g.firePoint = { curX, curY };
            }

            return g;
        }
    }

    Simulator::Simulator(const MissionInput& input,
                         const AmmoParameters& ammo,
                         const TargetDatabase& targets)
        : m_input(input), m_ammo(ammo), m_targets(targets) {}

    Simulator::TargetSolution
    Simulator::solveForTarget(int idx, float currentTime,
                              const Drone& drone, int currentTarget) const {
        const float arrayDt = m_input.getArrayTimeStep();
        const float accPath = m_input.getAccelerationPath();

        const BallisticResult bl = BallisticCalculator::compute(
            m_input.getAttackSpeed(), drone.zd(), m_ammo
        );

        Point2D tNow = m_targets.positionAt(idx, currentTime, arrayDt);
        DropGeometry g = computeDropGeometry(
            drone.position(), tNow, bl.horizontalDistance, accPath
        );
        const float droneTime = estimateBySimulation(
            drone,
            g.firePoint,
            idx,
            m_input.getSimTimeStep()
        );

        Point2D tNext = m_targets.positionAt(idx, currentTime + arrayDt, arrayDt);
        const float vx = (tNext.x - tNow.x) / arrayDt;
        const float vy = (tNext.y - tNow.y) / arrayDt;
        const float predictDt = droneTime + bl.timeOfFlight;

        Point2D predicted{
            tNow.x + vx * predictDt,
            tNow.y + vy * predictDt
        };

        g = computeDropGeometry(
            drone.position(), predicted, bl.horizontalDistance, accPath
        );

        if (g.hasIntermediate && idx == currentTarget &&
            (drone.state() == DroneState::Moving ||
             drone.state() == DroneState::Accelerating)) {
            g.hasIntermediate = false;
        }

        if (!g.hasIntermediate) {
            const float dx = predicted.x - drone.position().x;
            const float dy = predicted.y - drone.position().y;
            const float D = std::sqrt(dx * dx + dy * dy);

            if (D > 1e-6f) {
                const float ratio = (D - bl.horizontalDistance) / D;
                g.firePoint = {
                    drone.position().x + dx * ratio,
                    drone.position().y + dy * ratio
                };
            }
        }

        float totalTime = 0.0f;
        if (g.hasIntermediate) {
            totalTime += drone.estimateTimeToFirePoint(g.intermediatePoint);

            const float dirToFire = std::atan2(
                g.firePoint.y - g.intermediatePoint.y,
                g.firePoint.x - g.intermediatePoint.x
            );
            const float dirFromDrone = std::atan2(
                g.intermediatePoint.y - drone.position().y,
                g.intermediatePoint.x - drone.position().x
            );

            totalTime += std::fabs(normalizeAngle(dirToFire - dirFromDrone))
                       / m_input.getAngularSpeed();

            const float dx2 = g.firePoint.x - g.intermediatePoint.x;
            const float dy2 = g.firePoint.y - g.intermediatePoint.y;
            totalTime += std::sqrt(dx2 * dx2 + dy2 * dy2)
                       / m_input.getAttackSpeed();
        } else {
            totalTime = drone.estimateTimeToFirePoint(g.firePoint);
        }

        TargetSolution sol;
        sol.targetIndex = idx;
        sol.firePoint = g.firePoint;
        sol.intermediatePoint = g.intermediatePoint;
        sol.hasIntermediate = g.hasIntermediate;
        sol.bombFallTime = bl.timeOfFlight;
        sol.horizontalDistance = bl.horizontalDistance;
        sol.droneTime = totalTime;
        sol.totalTime = totalTime + bl.timeOfFlight;
        sol.distToTarget = std::hypot(
            predicted.x - drone.position().x,
            predicted.y - drone.position().y
        );

        // Diagnostic-only values. They are printed later but do not affect behavior.
        sol.predictedTarget = predicted;
        sol.solutionTime = currentTime;
        sol.simulatedDroneTime = estimateBySimulation(
            drone,
            sol.firePoint,
            idx,
            m_input.getSimTimeStep()
        );
        sol.predictedImpactTime = currentTime + sol.totalTime;

        if (sol.hasIntermediate) {
            sol.movementGoal = sol.intermediatePoint;
            sol.allowDrop = false;
        } else {
            sol.movementGoal = sol.firePoint;
            sol.allowDrop = true;
        }

        return sol;
    }

    int Simulator::pickBestTarget(float currentTime, const Drone& drone,
                                  int currentTarget,
                                  TargetSolution& outSol) const {
        int bestIdx = -1;
        float bestTime = std::numeric_limits<float>::max();

        for (int i = 0; i < TargetDatabase::kNumTargets; ++i) {
            const TargetSolution s = solveForTarget(
                i, currentTime, drone, currentTarget
            );

            if (s.totalTime < bestTime) {
                bestTime = s.totalTime;
                bestIdx = i;
                outSol = s;
            }
        }

        return bestIdx;
    }

    float Simulator::estimateBySimulation(Drone droneCopy,
                                          Point2D goal,
                                          int targetIndex,
                                          float dt) const {
        constexpr int maxLocalSteps = 5000;

        const float threshold = m_input.getHitRadius();

        for (int i = 0; i < maxLocalSteps; ++i) {
            const float dist = std::hypot(
                goal.x - droneCopy.position().x,
                goal.y - droneCopy.position().y
            );

            if (dist <= threshold) {
                return static_cast<float>(i) * dt;
            }

            droneCopy.step(dt, goal, targetIndex, false);
        }

        return std::numeric_limits<float>::infinity();
    }

    SimulationLog Simulator::run() {
        Drone drone(
            Point2D{ m_input.getXd(), m_input.getYd() },
            m_input.getZd(),
            m_input.getInitialDir(),
            m_input.getAttackSpeed(),
            m_input.getAccelerationPath(),
            m_input.getAngularSpeed(),
            m_input.getTurnThreshold(),
            m_input.getHitRadius()
        );

        SimulationLog log;
        const float dt = m_input.getSimTimeStep();
        float currentTime = 0.0f;
        int currentTarget = -1;

        for (int step = 0; step < MAX_STEPS; ++step) {
            TargetSolution best;
            const int bestIdx = pickBestTarget(
                currentTime, drone, currentTarget, best
            );
            currentTarget = bestIdx;

            log.record({ drone.position(), drone.direction(),
                         drone.state(), bestIdx });

            const bool dropped = drone.step(
                dt, best.movementGoal, bestIdx, best.allowDrop
            );

            if (dropped) {
                const float dropTime = currentTime + dt;
                const float impactTime = dropTime + best.bombFallTime;
                const float estimatedDropTime = best.solutionTime + best.droneTime;
                const float estimatedImpactTime = estimatedDropTime + best.bombFallTime;

                const Point2D targetAtDrop =
                    m_targets.positionAt(bestIdx, dropTime, m_input.getArrayTimeStep());

                const Point2D targetAtImpact =
                    m_targets.positionAt(bestIdx, impactTime, m_input.getArrayTimeStep());

                const Point2D targetAtPredictedImpact =
                    m_targets.positionAt(bestIdx,
                                         best.predictedImpactTime,
                                         m_input.getArrayTimeStep());

                const float dropTimingError = dropTime - estimatedDropTime;
                const float impactTimingError = impactTime - estimatedImpactTime;
                const float predictionErrorAtActualImpact = std::hypot(
                    best.predictedTarget.x - targetAtImpact.x,
                    best.predictedTarget.y - targetAtImpact.y
                );
                const float predictionErrorAtEstimatedImpact = std::hypot(
                    best.predictedTarget.x - targetAtPredictedImpact.x,
                    best.predictedTarget.y - targetAtPredictedImpact.y
                );

                std::cout << "DEBUG EVENT\n";
                std::cout << "  dropTime: " << dropTime << '\n';
                std::cout << "  impactTime: " << impactTime << '\n';
                std::cout << "  selected target: " << bestIdx << '\n';
                std::cout << "  drone/drop position: "
                          << drone.position().x << ", "
                          << drone.position().y << '\n';
                std::cout << "  drone direction: " << drone.direction() << '\n';
                std::cout << "  planned firePoint: "
                          << best.firePoint.x << ", "
                          << best.firePoint.y << '\n';
                std::cout << "  target at drop: "
                          << targetAtDrop.x << ", "
                          << targetAtDrop.y << '\n';
                std::cout << "  target at impact: "
                          << targetAtImpact.x << ", "
                          << targetAtImpact.y << '\n';
                std::cout << "  predicted target used by solution: "
                          << best.predictedTarget.x << ", "
                          << best.predictedTarget.y << '\n';
                std::cout << "  target at predicted impact time: "
                          << targetAtPredictedImpact.x << ", "
                          << targetAtPredictedImpact.y << '\n';
                std::cout << "  solution time: " << best.solutionTime << '\n';
                std::cout << "  estimated drone time remaining: "
                          << best.droneTime << '\n';
                std::cout << "  simulated drone time estimate: "
                          << best.simulatedDroneTime << '\n';
                std::cout << "  analytic-simulated drone time diff: "
                          << (best.droneTime - best.simulatedDroneTime) << '\n';
                std::cout << "  estimated drop time: " << estimatedDropTime << '\n';
                std::cout << "  drop timing error: " << dropTimingError << '\n';
                std::cout << "  estimated impact time: " << estimatedImpactTime << '\n';
                std::cout << "  impact timing error: " << impactTimingError << '\n';
                std::cout << "  prediction error at actual impact: "
                          << predictionErrorAtActualImpact << '\n';
                std::cout << "  prediction error at estimated impact: "
                          << predictionErrorAtEstimatedImpact << '\n';
                std::cout << "  distance drone-to-firePoint: "
                          << std::hypot(best.firePoint.x - drone.position().x,
                                        best.firePoint.y - drone.position().y)
                          << '\n';

                log.recordDrop({ dropTime, drone.position(),
                                 drone.direction(), bestIdx });
                break;
            }

            currentTime += dt;
        }

        return log;
    }

    float Simulator::normalizeAngle(float a) {
        while (a >  PI) a -= TWO_PI;
        while (a < -PI) a += TWO_PI;
        return a;
    }

}
