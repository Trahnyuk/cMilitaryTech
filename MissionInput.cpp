#include "MissionInput.h"

#include <fstream>
#include <stdexcept>

using std::string;
using std::ifstream;
using std::runtime_error;

namespace cMiltech {

    MissionInput::MissionInput(const string& filePath) {
        ifstream file(filePath);
        if (!file.is_open()) {
            throw runtime_error("Could not open input file: " + filePath);
        }

        if (!(file >> m_xd >> m_yd >> m_zd
                   >> m_targetX >> m_targetY
                   >> m_attackSpeed >> m_accelerationPath
                   >> m_ammoName)) {
            throw runtime_error("Malformed input file: " + filePath);
        }

        if (m_zd <= 0.0f) {
            throw runtime_error("Drone altitude (zd) must be positive.");
        }
        if (m_attackSpeed <= 0.0f) {
            throw runtime_error("Attack speed must be positive.");
        }
        if (m_accelerationPath < 0.0f) {
            throw runtime_error("Acceleration path cannot be negative.");
        }
    }

    float              MissionInput::getXd()               const noexcept { return m_xd; }
    float              MissionInput::getYd()               const noexcept { return m_yd; }
    float              MissionInput::getZd()               const noexcept { return m_zd; }
    float              MissionInput::getTargetX()          const noexcept { return m_targetX; }
    float              MissionInput::getTargetY()          const noexcept { return m_targetY; }
    float              MissionInput::getAttackSpeed()      const noexcept { return m_attackSpeed; }
    float              MissionInput::getAccelerationPath() const noexcept { return m_accelerationPath; }
    const std::string& MissionInput::getAmmoName()         const noexcept { return m_ammoName; }

}
