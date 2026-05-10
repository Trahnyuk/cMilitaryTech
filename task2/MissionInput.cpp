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
                   >> m_initialDir
                   >> m_attackSpeed >> m_accelerationPath
                   >> m_ammoName
                   >> m_arrayTimeStep >> m_simTimeStep
                   >> m_hitRadius
                   >> m_angularSpeed >> m_turnThreshold)) {
            throw runtime_error("Malformed input file: " + filePath);
        }

        if (m_zd <= 0.0f)             throw runtime_error("Drone altitude (zd) must be positive.");
        if (m_attackSpeed <= 0.0f)    throw runtime_error("Attack speed must be positive.");
        if (m_accelerationPath <= 0.0f) throw runtime_error("Acceleration path must be positive.");
        if (m_arrayTimeStep <= 0.0f)  throw runtime_error("arrayTimeStep must be positive.");
        if (m_simTimeStep   <= 0.0f)  throw runtime_error("simTimeStep must be positive.");
        if (m_hitRadius     <  0.0f)  throw runtime_error("hitRadius cannot be negative.");
        if (m_angularSpeed  <= 0.0f)  throw runtime_error("angularSpeed must be positive.");
        if (m_turnThreshold <  0.0f)  throw runtime_error("turnThreshold cannot be negative.");
    }

    float              MissionInput::getXd()               const noexcept { return m_xd; }
    float              MissionInput::getYd()               const noexcept { return m_yd; }
    float              MissionInput::getZd()               const noexcept { return m_zd; }
    float              MissionInput::getInitialDir()       const noexcept { return m_initialDir; }
    float              MissionInput::getAttackSpeed()      const noexcept { return m_attackSpeed; }
    float              MissionInput::getAccelerationPath() const noexcept { return m_accelerationPath; }
    const std::string& MissionInput::getAmmoName()         const noexcept { return m_ammoName; }
    float              MissionInput::getArrayTimeStep()    const noexcept { return m_arrayTimeStep; }
    float              MissionInput::getSimTimeStep()      const noexcept { return m_simTimeStep; }
    float              MissionInput::getHitRadius()        const noexcept { return m_hitRadius; }
    float              MissionInput::getAngularSpeed()     const noexcept { return m_angularSpeed; }
    float              MissionInput::getTurnThreshold()    const noexcept { return m_turnThreshold; }

}
