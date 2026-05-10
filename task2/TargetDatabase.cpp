#include "TargetDatabase.h"

#include <cmath>
#include <fstream>
#include <stdexcept>

namespace cMiltech {

    TargetDatabase::TargetDatabase(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open targets file: " + filePath);
        }

        // First 5 rows: X coords (60 floats per row).
        for (int i = 0; i < kNumTargets; ++i) {
            for (int j = 0; j < kNumSteps; ++j) {
                if (!(file >> m_x[i][j])) {
                    throw std::runtime_error("Malformed targets file (X data): " + filePath);
                }
            }
        }
        // Next 5 rows: Y coords.
        for (int i = 0; i < kNumTargets; ++i) {
            for (int j = 0; j < kNumSteps; ++j) {
                if (!(file >> m_y[i][j])) {
                    throw std::runtime_error("Malformed targets file (Y data): " + filePath);
                }
            }
        }
    }

    Point2D TargetDatabase::positionAt(int i, float t, float arrayTimeStep) const {
        if (i < 0 || i >= kNumTargets) {
            throw std::out_of_range("Invalid target index");
        }
        if (arrayTimeStep <= 0.0f) {
            throw std::invalid_argument("arrayTimeStep must be positive");
        }

        // Wrap t into [0, period) so negative or large times stay valid.
        const float period = arrayTimeStep * static_cast<float>(kNumSteps);
        float tMod = std::fmod(t, period);
        if (tMod < 0.0f) tMod += period;

        const int idx  = static_cast<int>(std::floor(tMod / arrayTimeStep)) % kNumSteps;
        const int next = (idx + 1) % kNumSteps;
        const float frac = (tMod - static_cast<float>(idx) * arrayTimeStep) / arrayTimeStep;

        return Point2D{
            m_x[i][idx] + (m_x[i][next] - m_x[i][idx]) * frac,
            m_y[i][idx] + (m_y[i][next] - m_y[i][idx]) * frac
        };
    }

}
