#define _CRT_SECURE_NO_WARNINGS  // Disable Windows strncpy warnings
#include "AmmoParameters.h"

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <cstring>

using std::cout;
using std::fixed;
using std::setprecision;
using std::invalid_argument;

namespace cMiltech {

    AmmoParameters::AmmoParameters()
        : m_mass(0.0f)
        , m_drag(0.0f)
        , m_lift(0.0f)
        , m_type(AmmoType::Standard) {
        m_name[0] = '\0';
    }

    AmmoParameters::AmmoParameters(const char* name,
                                   float mass,
                                   float drag,
                                   float lift,
                                   AmmoType type)
        : m_mass(mass)
        , m_drag(drag)
        , m_lift(lift)
        , m_type(type)
    {
        if (!name || name[0] == '\0') {
            throw invalid_argument("Ammo name cannot be empty.");
        }
        
        std::strncpy(m_name, name, 31);
        m_name[31] = '\0';
        
        validatePositive(m_mass, "mass");
        validatePositive(m_drag, "drag");

        if (m_lift < 0.0f) {
            throw invalid_argument("Field 'lift' cannot be negative.");
        }
    }

    // --- Getters ---

    const char* AmmoParameters::getName() const noexcept  { return m_name; }
    float       AmmoParameters::getMass() const noexcept  { return m_mass; }
    float       AmmoParameters::getDrag() const noexcept  { return m_drag; }
    float       AmmoParameters::getLift() const noexcept  { return m_lift; }
    AmmoType    AmmoParameters::getType() const noexcept  { return m_type; }

    bool AmmoParameters::isGliding() const noexcept {
        return m_type == AmmoType::Gliding;
    }

    // --- Display ---

    void AmmoParameters::print() const {
        cout << "Name:      " << m_name                              << '\n'
             << "Mass:      " << fixed << setprecision(2) << m_mass  << " kg\n"
             << "Drag:      " << fixed << setprecision(2) << m_drag  << '\n'
             << "Lift:      " << fixed << setprecision(1) << m_lift  << '\n'
             << "Gliding:   " << (isGliding() ? "Yes" : "No")       << '\n';
    }

    void AmmoParameters::validatePositive(float value, const char* field) {
        if (value <= 0.0f) {
            throw invalid_argument("Field must be positive.");
        }
    }

} 
