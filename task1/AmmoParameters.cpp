#include "AmmoParameters.h"

#include <iostream>
#include <iomanip>
#include <stdexcept>

using std::string;
using std::cout;
using std::fixed;
using std::setprecision;
using std::invalid_argument;

namespace cMiltech {

    AmmoParameters::AmmoParameters(string name,
                                   float mass,
                                   float drag,
                                   float lift,
                                   AmmoType type)
        : m_name(std::move(name))
        , m_mass(mass)
        , m_drag(drag)
        , m_lift(lift)
        , m_type(type)
    {
        if (m_name.empty()) {
            throw invalid_argument("Ammo name cannot be empty.");
        }
        validatePositive(m_mass, "mass");
        validatePositive(m_drag, "drag");

        if (m_lift < 0.0f) {
            throw invalid_argument("Field 'lift' cannot be negative.");
        }
    }

    // --- Getters ---

    const string& AmmoParameters::getName() const noexcept  { return m_name; }
    float         AmmoParameters::getMass() const noexcept  { return m_mass; }
    float         AmmoParameters::getDrag() const noexcept  { return m_drag; }
    float         AmmoParameters::getLift() const noexcept  { return m_lift; }
    AmmoType      AmmoParameters::getType() const noexcept  { return m_type; }

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


    void AmmoParameters::validatePositive(float value, const string& field) {
        if (value <= 0.0f) {
            throw invalid_argument("Field '" + field + "' must be positive.");
        }
    }

} 
