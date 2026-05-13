#define _CRT_SECURE_NO_WARNINGS
#include "SimulationLog.h"
#include "LoggingMacros.h"

#include <fstream>
#include <iomanip>
#include <stdexcept>
#include "json.hpp"

using json = nlohmann::json;

namespace cMiltech {

    SimulationLog::SimulationLog()
        : m_steps(nullptr)
        , m_stepCount(0)
        , m_stepCapacity(0)
        , m_hasDrop(false) {
    }

    SimulationLog::~SimulationLog() {
        delete[] m_steps;
        m_steps = nullptr;
    }

    // Move constructor
    SimulationLog::SimulationLog(SimulationLog&& other) noexcept
        : m_steps(other.m_steps)
        , m_stepCount(other.m_stepCount)
        , m_stepCapacity(other.m_stepCapacity)
        , m_drop(other.m_drop)
        , m_hasDrop(other.m_hasDrop)
    {
        other.m_steps = nullptr;
        other.m_stepCount = 0;
        other.m_stepCapacity = 0;
        other.m_hasDrop = false;
    }

    // Move assignment
    SimulationLog& SimulationLog::operator=(SimulationLog&& other) noexcept {
        if (this != &other) {
            delete[] m_steps;
            
            m_steps = other.m_steps;
            m_stepCount = other.m_stepCount;
            m_stepCapacity = other.m_stepCapacity;
            m_drop = other.m_drop;
            m_hasDrop = other.m_hasDrop;
            
            other.m_steps = nullptr;
            other.m_stepCount = 0;
            other.m_stepCapacity = 0;
            other.m_hasDrop = false;
        }
        return *this;
    }

    void SimulationLog::ensureCapacity() {
        if (m_stepCount >= m_stepCapacity) {
            int newCapacity = (m_stepCapacity == 0) ? 100 : m_stepCapacity * 2;
            
            Step* newSteps = new Step[newCapacity];
            
            for (int i = 0; i < m_stepCount; ++i) {
                newSteps[i] = m_steps[i];
            }
            
            delete[] m_steps;
            m_steps = newSteps;
            m_stepCapacity = newCapacity;
            
            DEBUG("Resized step array to capacity " << newCapacity);
        }
    }

    void SimulationLog::record(const Step& s) {
        ensureCapacity();
        m_steps[m_stepCount] = s;
        m_stepCount++;
    }

    void SimulationLog::writeToFile(const char* path) const {
        LOG("Writing simulation results to: " << path);

        json output;
        output["totalSteps"] = m_stepCount;
        output["steps"] = json::array();

        for (int i = 0; i < m_stepCount; ++i) {
            const Step& step = m_steps[i];
            json stepJson;
            stepJson["position"] = {{"x", step.pos.x}, {"y", step.pos.y}};
            stepJson["direction"] = step.direction;
            stepJson["state"] = static_cast<int>(step.state);
            stepJson["targetIndex"] = step.targetIndex;
            stepJson["dropPoint"] = {{"x", step.dropPoint.x}, {"y", step.dropPoint.y}};
            stepJson["aimPoint"] = {{"x", step.aimPoint.x}, {"y", step.aimPoint.y}};
            stepJson["predictedTarget"] = {{"x", step.predictedTarget.x}, 
                                          {"y", step.predictedTarget.y}};
            
            output["steps"].push_back(stepJson);
        }

        if (m_hasDrop) {
            output["dropEvent"] = {
                {"time", m_drop.time},
                {"position", {{"x", m_drop.dronePos.x}, {"y", m_drop.dronePos.y}}},
                {"direction", m_drop.direction},
                {"targetIndex", m_drop.targetIndex}
            };
        }

        std::ofstream outFile(path);
        if (!outFile.is_open()) {
            throw std::runtime_error("Could not open output file");
        }

        outFile << output.dump(2);
        LOG("Wrote " << m_stepCount << " steps to " << path);
    }

    void SimulationLog::writeToTextFile(const char* path) const {
        LOG("Writing simulation results (text format) to: " << path);

        std::ofstream out(path);
        if (!out.is_open())
            throw std::runtime_error("Could not open output file");

        out << std::fixed << std::setprecision(6);

        out << m_stepCount << '\n';

        for (int i = 0; i < m_stepCount; ++i) {
            if (i > 0) out << ' ';
            out << m_steps[i].pos.x << ' ' << m_steps[i].pos.y;
        }
        out << '\n';

        for (int i = 0; i < m_stepCount; ++i) {
            if (i > 0) out << ' ';
            out << m_steps[i].direction;
        }
        out << '\n';

        for (int i = 0; i < m_stepCount; ++i) {
            if (i > 0) out << ' ';
            out << static_cast<int>(m_steps[i].state);
        }
        out << '\n';

        for (int i = 0; i < m_stepCount; ++i) {
            if (i > 0) out << ' ';
            out << m_steps[i].targetIndex;
        }
        out << '\n';

        LOG("Wrote " << m_stepCount << " steps (text format)");
    }

}
