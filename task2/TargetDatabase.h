#ifndef TARGETDATABASE_H
#define TARGETDATABASE_H

#include "Geometry.h"

#include <array>
#include <string>

namespace cMiltech {

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
