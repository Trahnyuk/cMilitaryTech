#define _CRT_SECURE_NO_WARNINGS  // Disable Windows strncpy warnings
#include "MissionInput.h"
#include "LoggingMacros.h"

#include <fstream>
#include <stdexcept>
#include <cstring>
#include <cmath>
#include "json.hpp"

using json = nlohmann::json;

namespace cMiltech {

    MissionInput::MissionInput(const char* configJsonPath) {
        m_ammoName[0] = '\0';
        loadFromJson(configJsonPath);
    }

    void MissionInput::loadFromJson(const char* configJsonPath) {
        LOG("Loading config from: " << configJsonPath);

        std::ifstream file(configJsonPath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open config file");
        }

        json config;
        try {
            file >> config;
        } catch (const std::exception&) {
            throw std::runtime_error("Failed to parse JSON config");
        }

        // Parse drone configuration
        try {
            m_xd = config["drone"]["position"]["x"];
            m_yd = config["drone"]["position"]["y"];
            m_zd = config["drone"]["altitude"];
            
            // Initial direction is in degrees in JSON, convert to radians
            float dirDegrees = config["drone"]["initialDirection"];
            m_initialDir = dirDegrees * 3.14159265f / 180.0f;
            
            m_attackSpeed = config["drone"]["attackSpeed"];
            m_accelerationPath = config["drone"]["accelerationPath"];
            m_angularSpeed = config["drone"]["angularSpeed"];
            m_turnThreshold = config["drone"]["turnThreshold"];

            // Ammo name
            auto ammoStr = config["ammo"].get<std::string>();
            std::strncpy(m_ammoName, ammoStr.c_str(), 31);
            m_ammoName[31] = '\0';

            // Simulation parameters
            m_simTimeStep = config["simulation"]["timeStep"];
            m_hitRadius = config["simulation"]["hitRadius"];
            m_arrayTimeStep = config["targetArrayTimeStep"];

        } catch (const std::exception&) {
            throw std::runtime_error("Missing or invalid field in config JSON");
        }

        // Validation
        if (m_zd <= 0.0f)             
            throw std::runtime_error("Drone altitude must be positive.");
        if (m_attackSpeed <= 0.0f)    
            throw std::runtime_error("Attack speed must be positive.");
        if (m_accelerationPath <= 0.0f) 
            throw std::runtime_error("Acceleration path must be positive.");
        if (m_arrayTimeStep <= 0.0f)  
            throw std::runtime_error("arrayTimeStep must be positive.");
        if (m_simTimeStep   <= 0.0f)  
            throw std::runtime_error("simTimeStep must be positive.");
        if (m_hitRadius     <  0.0f)  
            throw std::runtime_error("hitRadius cannot be negative.");
        if (m_angularSpeed  <= 0.0f)  
            throw std::runtime_error("angularSpeed must be positive.");
        if (m_turnThreshold <  0.0f)  
            throw std::runtime_error("turnThreshold cannot be negative.");

        LOG("Config loaded successfully: " << m_ammoName << ", speed=" << m_attackSpeed);
    }

    float MissionInput::getXd() const noexcept { return m_xd; }
    float MissionInput::getYd() const noexcept { return m_yd; }
    float MissionInput::getZd() const noexcept { return m_zd; }
    float MissionInput::getInitialDir() const noexcept { return m_initialDir; }
    float MissionInput::getAttackSpeed() const noexcept { return m_attackSpeed; }
    float MissionInput::getAccelerationPath() const noexcept { return m_accelerationPath; }
    const char* MissionInput::getAmmoName() const noexcept { return m_ammoName; }
    float MissionInput::getArrayTimeStep() const noexcept { return m_arrayTimeStep; }
    float MissionInput::getSimTimeStep() const noexcept { return m_simTimeStep; }
    float MissionInput::getHitRadius() const noexcept { return m_hitRadius; }
    float MissionInput::getAngularSpeed() const noexcept { return m_angularSpeed; }
    float MissionInput::getTurnThreshold() const noexcept { return m_turnThreshold; }

}
