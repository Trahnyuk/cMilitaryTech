#ifndef AMMOPARAMETERS_H
#define AMMOPARAMETERS_H

namespace cMiltech {

    enum class AmmoType {
        Standard,
        Gliding
    };

    class AmmoParameters {
    public:

        AmmoParameters();
        AmmoParameters(const char* name,
                       float mass,
                       float drag,
                       float lift,
                       AmmoType type);

        // Getters
        const char* getName()  const noexcept;
        float       getMass()  const noexcept;
        float       getDrag()  const noexcept;
        float       getLift()  const noexcept;
        AmmoType    getType()  const noexcept;
        bool        isGliding() const noexcept;

        void print() const;

    private:
        char     m_name[32];
        float    m_mass;
        float    m_drag;
        float    m_lift;
        AmmoType m_type;

        static void validatePositive(float value, const char* field);
    };

}

#endif