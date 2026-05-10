#include "SimulationLog.h"

#include <fstream>
#include <iomanip>
#include <stdexcept>

namespace cMiltech {

    void SimulationLog::writeToFile(const std::string& path) const {
        std::ofstream out(path);
        if (!out.is_open())
            throw std::runtime_error("Could not open output file: " + path);

        out << std::fixed << std::setprecision(6);

        // Line 1: N
        out << m_steps.size() << '\n';

        // Line 2: x0 y0 x1 y1 ...
        for (std::size_t i = 0; i < m_steps.size(); ++i) {
            if (i > 0) out << ' ';
            out << m_steps[i].pos.x << ' ' << m_steps[i].pos.y;
        }
        out << '\n';

        // Line 3: directions (radians)
        for (std::size_t i = 0; i < m_steps.size(); ++i) {
            if (i > 0) out << ' ';
            out << m_steps[i].direction;
        }
        out << '\n';

        // Line 4: states 0..4
        for (std::size_t i = 0; i < m_steps.size(); ++i) {
            if (i > 0) out << ' ';
            out << static_cast<int>(m_steps[i].state);
        }
        out << '\n';

        // Line 5: target indices
        for (std::size_t i = 0; i < m_steps.size(); ++i) {
            if (i > 0) out << ' ';
            out << m_steps[i].targetIndex;
        }
        out << '\n';
    }

}
