#ifndef TARGETDATABASE_H
#define TARGETDATABASE_H

#include "Geometry.h"

#include <array>
#include <string>

namespace cMiltech {

    // Stores 5 moving targets, each with 60 sampled (x,y) positions.
    // The array is cyclic in time — index 60 wraps to index 0 (HW2 §4).
    class TargetDatabase {
    public:
        static constexpr int kNumTargets = 5;
        static constexpr int kNumSteps   = 60;

        explicit TargetDatabase(const std::string& filePath);

        // Linearly interpolated position of target i at time t,
        // given the spacing between stored samples.
        Point2D positionAt(int targetIndex, float t, float arrayTimeStep) const;

        int targetCount() const noexcept { return kNumTargets; }

    private:
        std::array<std::array<float, kNumSteps>, kNumTargets> m_x{};
        std::array<std::array<float, kNumSteps>, kNumTargets> m_y{};
    };

}

#endif // TARGETDATABASE_H
