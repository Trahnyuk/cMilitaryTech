#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "AmmoParameters.h"
#include "BallisticResult.h"
#include "Drone.h"
#include "Geometry.h"
#include "MissionInput.h"
#include "SimulationLog.h"
#include "TargetDatabase.h"

namespace cMiltech {

    class Simulator {
    public:
        static constexpr int MAX_STEPS = 10000;

        Simulator(const MissionInput&   input,
                  const AmmoParameters& ammo,
                  const TargetDatabase& targets);

        SimulationLog run();

    private:
        struct TargetSolution {
            int     targetIndex{-1};

          
            Point2D movementGoal{};
            Point2D firePoint{};
            Point2D intermediatePoint{};
            bool    hasIntermediate{false};
            bool    allowDrop{false};

            float   bombFallTime{};
            float   horizontalDistance{};
            float   droneTime{};
            float   totalTime{};
            float   distToTarget{};

            Point2D predictedTarget{};
            float   solutionTime{};
            float   simulatedDroneTime{};
            float   predictedImpactTime{};
        };

        TargetSolution solveForTarget(int targetIndex,
                                      float currentTime,
                                      const Drone& drone,
                                      int currentTarget) const;

        int pickBestTarget(float currentTime,
                           const Drone& drone,
                           int currentTarget,
                           TargetSolution& outSol) const;

        float estimateBySimulation(Drone droneCopy,
                                   Point2D goal,
                                   int targetIndex,
                                   float dt) const;

        static float normalizeAngle(float a);

        const MissionInput&   m_input;
        const AmmoParameters& m_ammo;
        const TargetDatabase& m_targets;
    };

}

#endif
