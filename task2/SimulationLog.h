#ifndef SIMULATIONLOG_H
#define SIMULATIONLOG_H

#include "DroneState.h"
#include "Geometry.h"

#include <string>
#include <vector>

namespace cMiltech {

    // Records per-step drone state and the single drop event.
    class SimulationLog {
    public:
        struct Step {
            Point2D    pos;
            float      direction;
            DroneState state;
            int        targetIndex;
        };

        struct DropEvent {
            float   time;
            Point2D dronePos;
            float   direction;
            int     targetIndex;
        };

        void record(const Step& s)          { m_steps.push_back(s); }
        void recordDrop(const DropEvent& d) { m_drop = d; m_hasDrop = true; }

        std::size_t size()    const noexcept { return m_steps.size(); }
        bool        hasDrop() const noexcept { return m_hasDrop; }
        const DropEvent& drop() const noexcept { return m_drop; }

        // Output format (HW2 §8):
        //   Line 1: N
        //   Line 2: x0 y0 x1 y1 ...
        //   Line 3: directions
        //   Line 4: states (0..4)
        //   Line 5: target indices
        void writeToFile(const std::string& path) const;

    private:
        std::vector<Step> m_steps;
        DropEvent         m_drop{};
        bool              m_hasDrop{false};
    };

}

#endif // SIMULATIONLOG_H
