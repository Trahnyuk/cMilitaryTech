#ifndef MISSIONINPUT_H
#define MISSIONINPUT_H

namespace cMiltech {

    class MissionInput {
    public:
        // Constructor takes config.json path as C-string
        explicit MissionInput(const char* configJsonPath);

        // Drone position
        float getXd() const noexcept;
        float getYd() const noexcept;
        float getZd() const noexcept;

        // Initial heading (radians from +X axis)
        float getInitialDir() const noexcept;

        // Flight parameters
        float getAttackSpeed()      const noexcept;
        float getAccelerationPath() const noexcept;

        // Ammo identifier (C-string)
        const char* getAmmoName() const noexcept;

        // Simulation parameters
        float getArrayTimeStep() const noexcept;
        float getSimTimeStep()   const noexcept;
        float getHitRadius()     const noexcept;
        float getAngularSpeed()  const noexcept;
        float getTurnThreshold() const noexcept;

    private:
        void loadFromJson(const char* configJsonPath);

        float m_xd{};
        float m_yd{};
        float m_zd{};
        float m_initialDir{};
        float m_attackSpeed{};
        float m_accelerationPath{};
        char  m_ammoName[32];
        float m_arrayTimeStep{};
        float m_simTimeStep{};
        float m_hitRadius{};
        float m_angularSpeed{};
        float m_turnThreshold{};
    };

}

#endif
