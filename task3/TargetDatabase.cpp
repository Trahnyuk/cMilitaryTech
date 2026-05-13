#include "TargetDatabase.h"
#include "LoggingMacros.h"

#include <cmath>
#include <fstream>
#include <stdexcept>
#include "json.hpp"

using json = nlohmann::json;

namespace cMiltech {

    TargetDatabase::TargetDatabase(const char* targetsJsonPath)
        : m_targets(nullptr), m_targetCount(0), m_timeSteps(0) {
        loadFromJson(targetsJsonPath);
    }

    TargetDatabase::~TargetDatabase() {
        cleanup();
    }

    TargetDatabase::TargetDatabase(const TargetDatabase& other)
        : m_targets(nullptr), m_targetCount(0), m_timeSteps(0) {
        copyFrom(other);
    }

    TargetDatabase& TargetDatabase::operator=(const TargetDatabase& other) {
        if (this != &other) {
            cleanup();
            copyFrom(other);
        }
        return *this;
    }

    void TargetDatabase::cleanup() {
        if (m_targets) {
            
            for (int i = 0; i < m_targetCount; ++i) {
                delete[] m_targets[i];
            }
            delete[] m_targets;
            m_targets = nullptr;
        }
        m_targetCount = 0;
        m_timeSteps = 0;
    }

    void TargetDatabase::copyFrom(const TargetDatabase& other) {
        m_targetCount = other.m_targetCount;
        m_timeSteps = other.m_timeSteps;

        if (m_targetCount > 0 && m_timeSteps > 0) {
            
            m_targets = new Point2D*[m_targetCount];
            for (int i = 0; i < m_targetCount; ++i) {
                m_targets[i] = new Point2D[m_timeSteps];
                for (int j = 0; j < m_timeSteps; ++j) {
                    m_targets[i][j] = other.m_targets[i][j];
                }
            }
        }
    }

    void TargetDatabase::loadFromJson(const char* targetsJsonPath) {
        LOG("Loading target database from: " << targetsJsonPath);

        std::ifstream file(targetsJsonPath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open targets file");
        }

        json targetsJson;
        try {
            file >> targetsJson;
        } catch (const std::exception&) {
            throw std::runtime_error("Failed to parse targets JSON");
        }

        try {
            m_targetCount = targetsJson["targetCount"];
            m_timeSteps = targetsJson["timeSteps"];

            if (m_targetCount <= 0 || m_timeSteps <= 0) {
                throw std::runtime_error("Invalid targetCount or timeSteps");
            }

            const auto& targetsArray = targetsJson["targets"];
            if (targetsArray.size() != static_cast<size_t>(m_targetCount)) {
                throw std::runtime_error("targetCount doesn't match array size");
            }

           
            m_targets = new Point2D*[m_targetCount];
            
            for (int i = 0; i < m_targetCount; ++i) {
                m_targets[i] = new Point2D[m_timeSteps];
                
                const auto& positions = targetsArray[i]["positions"];
                
                if (positions.size() != static_cast<size_t>(m_timeSteps)) {
                    
                    for (int k = 0; k <= i; ++k) {
                        delete[] m_targets[k];
                    }
                    delete[] m_targets;
                    m_targets = nullptr;
                    throw std::runtime_error("Target has incorrect number of positions");
                }

                for (int j = 0; j < m_timeSteps; ++j) {
                    m_targets[i][j].x = positions[j]["x"];
                    m_targets[i][j].y = positions[j]["y"];
                }

                DEBUG("Loaded target " << i << " with " << m_timeSteps << " positions");
            }

            LOG("Loaded " << m_targetCount << " targets with " 
                << m_timeSteps << " time steps each");

        } catch (const std::exception&) {
            cleanup();
            throw std::runtime_error("Error parsing targets JSON");
        }
    }

    Point2D TargetDatabase::positionAt(int targetIndex, float t, float arrayTimeStep) const {
        if (targetIndex < 0 || targetIndex >= m_targetCount) {
            throw std::out_of_range("Invalid target index");
        }
        if (arrayTimeStep <= 0.0f) {
            throw std::invalid_argument("arrayTimeStep must be positive");
        }

        
        const float period = arrayTimeStep * static_cast<float>(m_timeSteps);
        float tMod = std::fmod(t, period);
        if (tMod < 0.0f) tMod += period;

        const int idx = static_cast<int>(std::floor(tMod / arrayTimeStep)) % m_timeSteps;
        const int next = (idx + 1) % m_timeSteps;
        const float frac = (tMod - static_cast<float>(idx) * arrayTimeStep) / arrayTimeStep;

        
        const Point2D& p1 = m_targets[targetIndex][idx];
        const Point2D& p2 = m_targets[targetIndex][next];
        
        return p1 + (p2 - p1) * frac;
    }

}
