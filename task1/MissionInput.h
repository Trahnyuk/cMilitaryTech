#include <string>

namespace cMiltech {

    class MissionInput {
    public:
        
        explicit MissionInput(const std::string& filePath);

        // Drone position
        float getXd()   const noexcept;
        float getYd()   const noexcept;
        float getZd()   const noexcept;

        // Target position (ground, z = 0)
        float getTargetX() const noexcept;
        float getTargetY() const noexcept;

        // Flight parameters
        float getAttackSpeed()      const noexcept;
        float getAccelerationPath() const noexcept;

        // Ammo identifier
        const std::string& getAmmoName() const noexcept;

    private:
        float       m_xd;
        float       m_yd;
        float       m_zd;
        float       m_targetX;
        float       m_targetY;
        float       m_attackSpeed;
        float       m_accelerationPath;
        std::string m_ammoName;
    };

}
