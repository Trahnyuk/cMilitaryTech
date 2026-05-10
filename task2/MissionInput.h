#ifndef MISSIONINPUT_H
#define MISSIONINPUT_H

#include <string>

namespace cMiltech {

    // Parses the HW2 input.txt format:
    //   xd yd zd initialDir attackSpeed accelerationPath ammo_name
    //   arrayTimeStep simTimeStep hitRadius angularSpeed turnThreshold
    class MissionInput {
    public:
        explicit MissionInput(const std::string& filePath);

        // Drone position
        float getXd() const noexcept;
        float getYd() const noexcept;
        float getZd() const noexcept;

        // Initial heading (radians from +X axis)
        float getInitialDir() const noexcept;

        // Flight parameters
        float getAttackSpeed()      const noexcept;
        float getAccelerationPath() const noexcept;

        // Ammo identifier
        const std::string& getAmmoName() const noexcept;

        // Simulation parameters (HW2)
        float getArrayTimeStep() const noexcept;
        float getSimTimeStep()   const noexcept;
        float getHitRadius()     const noexcept;
        float getAngularSpeed()  const noexcept;
        float getTurnThreshold() const noexcept;

    private:
        float       m_xd{};
        float       m_yd{};
        float       m_zd{};
        float       m_initialDir{};
        float       m_attackSpeed{};
        float       m_accelerationPath{};
        std::string m_ammoName;
        float       m_arrayTimeStep{};
        float       m_simTimeStep{};
        float       m_hitRadius{};
        float       m_angularSpeed{};
        float       m_turnThreshold{};
    };

}

#endif // MISSIONINPUT_H
