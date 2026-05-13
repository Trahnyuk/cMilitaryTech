#ifndef TARGETDATABASE_H
#define TARGETDATABASE_H

#include "Geometry.h"

namespace cMiltech {

    class TargetDatabase {
    public:

        explicit TargetDatabase(const char* targetsJsonPath);

        ~TargetDatabase();
        TargetDatabase(const TargetDatabase& other);
        TargetDatabase& operator=(const TargetDatabase& other);

        Point2D positionAt(int targetIndex, float t, float arrayTimeStep) const;

        int targetCount() const noexcept { return m_targetCount; }
        int timeSteps() const noexcept { return m_timeSteps; }

    private:
        void loadFromJson(const char* targetsJsonPath);
        void cleanup();
        void copyFrom(const TargetDatabase& other);

        Point2D** m_targets;
        int m_targetCount;
        int m_timeSteps;
    };

}

#endif
