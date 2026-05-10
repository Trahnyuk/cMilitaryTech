#ifndef AMMOPARAMETERS_H
#define AMMOPARAMETERS_H

#include <string>

namespace cMiltech {

    enum class AmmoType {
        Standard,
        Gliding
    };

    class AmmoParameters {
    public:
        
        AmmoParameters(std::string name,
                       float mass,
                       float drag,
                       float lift,
                       AmmoType type);

    
        const std::string& getName()  const noexcept;
        float              getMass()  const noexcept;
        float              getDrag()  const noexcept;
        float              getLift()  const noexcept;
        AmmoType           getType()  const noexcept;
        bool               isGliding() const noexcept;

        void print() const;

    private:
        std::string m_name;
        float       m_mass;
        float       m_drag;
        float       m_lift;
        AmmoType    m_type;

        static void validatePositive(float value, const std::string& field);
    };

}

#endif // AMMOPARAMETERS_H
