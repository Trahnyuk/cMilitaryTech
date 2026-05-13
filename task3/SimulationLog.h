#ifndef SIMULATIONLOG_H
#define SIMULATIONLOG_H

#include "DroneState.h"
#include "Geometry.h"

namespace cMiltech {

    class SimulationLog {
    public:
        struct Step {
            Point2D    pos;
            float      direction;
            DroneState state;
            int        targetIndex;
            
            Point2D    dropPoint;
            Point2D    aimPoint;
            Point2D    predictedTarget;
        };

        struct DropEvent {
            float   time;
            Point2D dronePos;
            float   direction;
            int     targetIndex;
        };

        SimulationLog();
        ~SimulationLog();
        
        SimulationLog(SimulationLog&& other) noexcept;
        SimulationLog& operator=(SimulationLog&& other) noexcept;
        
        SimulationLog(const SimulationLog&) = delete;
        SimulationLog& operator=(const SimulationLog&) = delete;

        void record(const Step& s);
        void recordDrop(const DropEvent& d) { m_drop = d; m_hasDrop = true; }

        int size() const noexcept { return m_stepCount; }
        bool hasDrop() const noexcept { return m_hasDrop; }
        const DropEvent& drop() const noexcept { return m_drop; }

        void writeToFile(const char* path) const;
        void writeToTextFile(const char* path) const;

    private:
        void ensureCapacity();

        Step* m_steps;
        int m_stepCount;
        int m_stepCapacity;
        DropEvent m_drop{};
        bool m_hasDrop{false};
    };

}

#endif
