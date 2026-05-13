#define _USE_MATH_DEFINES
#include "Simulator.h"
#include "BallisticCalculator.h"
#include "LoggingMacros.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <cstring>

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
            
            // Using operator overloading
            Point2D delta = target - drone;
            const float D = length(delta);

            g.hasIntermediate = (hDist + accPath > D);

            Point2D current = drone;
            float distToTarget = D;

            if (g.hasIntermediate) {
                if (D < 1e-6f) {
                    current = {target.x - (hDist + accPath), target.y};
                    distToTarget = hDist + accPath;
                } else {

                    current = target - normalize(delta) * (hDist + accPath);
                    distToTarget = length(target - current);
                }
                g.intermediatePoint = current;
            }

            if (distToTarget > 1e-6f) {
                Point2D toTarget = target - current;
                g.firePoint = current + normalize(toTarget) * (distToTarget - hDist);
            } else {
                g.firePoint = current;
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
        
        const int maxStep = m_targets.timeSteps() - 1;
        const int currentStep = static_cast<int>(currentTime / arrayDt);
        const float dt = arrayDt;
        const float predictDt = droneTime + bl.timeOfFlight;
        
        Point2D predicted;
        
        const float FINAL_APPROACH_THRESHOLD = 15.0f;
        const float impactTime = currentTime + predictDt;
        const bool isFinalApproach = (predictDt < FINAL_APPROACH_THRESHOLD);
        const bool canUseFutureData = (impactTime / dt < m_targets.timeSteps() - 1);
        
        if (isFinalApproach && canUseFutureData) {
            predicted = m_targets.positionAt(idx, impactTime, arrayDt);
            
            DEBUG("Target " << idx << " prediction (FINAL APPROACH - actual):");
            DEBUG("  Predict time: " << predictDt << " s");
            DEBUG("  Impact time: " << impactTime << " s");
            DEBUG("  Predicted pos: (" << predicted.x << ", " << predicted.y << ")");
            
        } else {
            Point2D p0, p1, p2;
            float vx, vy, ax, ay;
            
            // Use 3-5 points depending on available history
            if (currentStep >= 4) {
                // Use 5 points for best accuracy
                Point2D pm2 = m_targets.positionAt(idx, (currentStep - 4) * arrayDt, arrayDt);
                Point2D pm1 = m_targets.positionAt(idx, (currentStep - 3) * arrayDt, arrayDt);
                p0 = m_targets.positionAt(idx, (currentStep - 2) * arrayDt, arrayDt);
                p1 = m_targets.positionAt(idx, (currentStep - 1) * arrayDt, arrayDt);
                p2 = m_targets.positionAt(idx, currentStep * arrayDt, arrayDt);
                
                // Calculate velocities at 4 points
                float v0x = (p0.x - pm1.x) / dt;
                float v0y = (p0.y - pm1.y) / dt;
                float v1x = (p1.x - p0.x) / dt;
                float v1y = (p1.y - p0.y) / dt;
                float v2x = (p2.x - p1.x) / dt;
                float v2y = (p2.y - p1.y) / dt;
                
                // Average the velocities for smoother prediction
                vx = (v0x + v1x + v2x) / 3.0f;
                vy = (v0y + v1y + v2y) / 3.0f;
                
                // Calculate acceleration (smoothed)
                float a0x = (v1x - v0x) / dt;
                float a0y = (v1y - v0y) / dt;
                float a1x = (v2x - v1x) / dt;
                float a1y = (v2y - v1y) / dt;
                
                ax = (a0x + a1x) / 2.0f;
                ay = (a0y + a1y) / 2.0f;
                
            } else if (currentStep >= 2) {
                // Use 3 points
                p0 = m_targets.positionAt(idx, (currentStep - 2) * arrayDt, arrayDt);
                p1 = m_targets.positionAt(idx, (currentStep - 1) * arrayDt, arrayDt);
                p2 = m_targets.positionAt(idx, currentStep * arrayDt, arrayDt);
                
                float v1x = (p1.x - p0.x) / dt;
                float v1y = (p1.y - p0.y) / dt;
                float v2x = (p2.x - p1.x) / dt;
                float v2y = (p2.y - p1.y) / dt;
                
                vx = v2x;
                vy = v2y;
                ax = (v2x - v1x) / dt;
                ay = (v2y - v1y) / dt;
                
            } else if (currentStep >= 1) {
                // Only 2 points: linear prediction
                p1 = m_targets.positionAt(idx, (currentStep - 1) * arrayDt, arrayDt);
                p2 = m_targets.positionAt(idx, currentStep * arrayDt, arrayDt);
                
                vx = (p2.x - p1.x) / dt;
                vy = (p2.y - p1.y) / dt;
                ax = 0.0f;
                ay = 0.0f;
                
            } else {
                // Beginning: use forward points
                p0 = m_targets.positionAt(idx, 0.0f, arrayDt);
                p1 = m_targets.positionAt(idx, arrayDt, arrayDt);
                p2 = m_targets.positionAt(idx, 2.0f * arrayDt, arrayDt);
                
                float v1x = (p1.x - p0.x) / dt;
                float v1y = (p1.y - p0.y) / dt;
                float v2x = (p2.x - p1.x) / dt;
                float v2y = (p2.y - p1.y) / dt;
                
                vx = v2x;
                vy = v2y;
                ax = (v2x - v1x) / dt;
                ay = (v2y - v1y) / dt;
            }
            
            // Predict future position using kinematic equation
            predicted = Point2D{
                p2.x + vx * predictDt + 0.5f * ax * predictDt * predictDt,
                p2.y + vy * predictDt + 0.5f * ay * predictDt * predictDt
            };
            
            DEBUG("Target " << idx << " prediction (LONG RANGE - kinematic):");
            DEBUG("  Current pos: (" << p2.x << ", " << p2.y << ")");
            DEBUG("  Velocity: (" << vx << ", " << vy << ") m/s");
            DEBUG("  Acceleration: (" << ax << ", " << ay << ") m/s²");
            DEBUG("  Predict time: " << predictDt << " s");
            DEBUG("  Predicted pos: (" << predicted.x << ", " << predicted.y << ")");
        }

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
        
        // Hysteresis: Current target gets a bonus to prevent thrashing
        const float SWITCH_THRESHOLD = 0.8f;
        for (int i = 0; i < m_targets.targetCount(); ++i) {
            const TargetSolution s = solveForTarget(
                i, currentTime, drone, currentTarget
            );
            
            float effectiveTime = s.totalTime;
            
            // Give current target a bonus to reduce switching
            if (i == currentTarget && currentTarget >= 0) {
                effectiveTime *= SWITCH_THRESHOLD;  // 20% bonus
                DEBUG("Target " << i << " (CURRENT): time=" << s.totalTime 
                      << " (effective=" << effectiveTime << " with bonus)");
            } else {
                DEBUG("Target " << i << ": time=" << s.totalTime);
            }

            if (effectiveTime < bestTime) {
                bestTime = (i == currentTarget) ? s.totalTime : effectiveTime;
                bestIdx = i;
                outSol = s;
            }
        }

        if (bestIdx != currentTarget && currentTarget >= 0) {
            LOG("Switching from target " << currentTarget << " to " << bestIdx 
                << " (time improvement: " << (outSol.totalTime / bestTime) << "x)");
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
        
        // Adaptive re-evaluation:
        const int REEVAL_INTERVAL_NORMAL = 40;
        const int REEVAL_INTERVAL_FINAL = 1; 
        int stepsSinceLastReeval = REEVAL_INTERVAL_NORMAL;

        LOG("Starting simulation...");

        for (int step = 0; step < MAX_STEPS; ++step) {
            TargetSolution best;
            int bestIdx;
            
            // Check close to fire point
            bool isFinalApproach = false;
            if (currentTarget >= 0) {
                TargetSolution current = solveForTarget(currentTarget, currentTime, drone, currentTarget);
                float distToFire = std::hypot(
                    current.firePoint.x - drone.position().x,
                    current.firePoint.y - drone.position().y
                );
                
                isFinalApproach = (distToFire < 80.0f);
            }
            
            // Adaptive re-evaluation interval
            int reeval_interval = isFinalApproach ? REEVAL_INTERVAL_FINAL : REEVAL_INTERVAL_NORMAL;
            
            if (stepsSinceLastReeval >= reeval_interval || currentTarget < 0) {
                bestIdx = pickBestTarget(
                    currentTime, drone, currentTarget, best
                );
                
                if (bestIdx != currentTarget) {
                    DEBUG("Step " << step << ": Re-evaluated, switching to target " << bestIdx);
                }
                
                currentTarget = bestIdx;
                stepsSinceLastReeval = 0;
            } else {
                bestIdx = currentTarget;
                best = solveForTarget(currentTarget, currentTime, drone, currentTarget);
                stepsSinceLastReeval++;
            }

            DEBUG("Step " << step << ": pos=(" << drone.position().x << "," 
                  << drone.position().y << "), target=" << bestIdx 
                  << ", state=" << static_cast<int>(drone.state()));

            // Calculate aim point 
            Point2D directionVec = {std::cos(drone.direction()), std::sin(drone.direction())};
            Point2D aimPoint = drone.position() + directionVec * best.horizontalDistance;

            // Record step
            log.record({ 
                drone.position(),
                drone.direction(),
                drone.state(),
                bestIdx,
                best.firePoint,
                aimPoint,
                best.predictedTarget
            });

            const bool dropped = drone.step(
                dt, best.movementGoal, bestIdx, best.allowDrop
            );

            if (dropped) {
                const float dropTime = currentTime + dt;
                const float impactTime = dropTime + best.bombFallTime;
                const float estimatedDropTime = best.solutionTime + best.droneTime;
                const float estimatedImpactTime = estimatedDropTime + best.bombFallTime;

                LOG("Bomb dropped at t=" << dropTime << "s on target " << bestIdx);

                const Point2D targetAtDrop =
                    m_targets.positionAt(bestIdx, dropTime, m_input.getArrayTimeStep());

                const Point2D targetAtImpact =
                    m_targets.positionAt(bestIdx, impactTime, m_input.getArrayTimeStep());

                const Point2D targetAtPredictedImpact =
                    m_targets.positionAt(bestIdx,
                                         best.predictedImpactTime,
                                         m_input.getArrayTimeStep());

                // Distance calculations
                const float dropTimingError = dropTime - estimatedDropTime;
                const float impactTimingError = impactTime - estimatedImpactTime;
                const float predictionErrorAtActualImpact = 
                    length(best.predictedTarget - targetAtImpact);
                const float predictionErrorAtEstimatedImpact = 
                    length(best.predictedTarget - targetAtPredictedImpact);

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
