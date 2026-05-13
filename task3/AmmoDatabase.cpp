#define _CRT_SECURE_NO_WARNINGS  // Disable Windows strncpy warnings
#include "AmmoDatabase.h"
#include "LoggingMacros.h"

#include <fstream>
#include <cstring>
#include <stdexcept>
#include "json.hpp"

using json = nlohmann::json;

namespace cMiltech {

    AmmoDatabase::AmmoDatabase(const char* ammoJsonPath)
        : m_entries(nullptr), m_count(0), m_capacity(0) {
        loadFromJson(ammoJsonPath);
    }

    AmmoDatabase::~AmmoDatabase() {
        cleanup();
    }

    AmmoDatabase::AmmoDatabase(const AmmoDatabase& other)
        : m_entries(nullptr), m_count(0), m_capacity(0) {
        copyFrom(other);
    }

    AmmoDatabase& AmmoDatabase::operator=(const AmmoDatabase& other) {
        if (this != &other) {
            cleanup();
            copyFrom(other);
        }
        return *this;
    }

    void AmmoDatabase::cleanup() {
        delete[] m_entries;
        m_entries = nullptr;
        m_count = 0;
        m_capacity = 0;
    }

    void AmmoDatabase::copyFrom(const AmmoDatabase& other) {
        m_count = other.m_count;
        m_capacity = other.m_capacity;
        
        if (m_capacity > 0) {
            m_entries = new AmmoParameters[m_capacity];
            for (int i = 0; i < m_count; ++i) {
                m_entries[i] = other.m_entries[i];
            }
        }
    }

    void AmmoDatabase::loadFromJson(const char* ammoJsonPath) {
        LOG("Loading ammunition database from: " << ammoJsonPath);

        std::ifstream file(ammoJsonPath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open ammo file");
        }

        json ammoArray;
        try {
            file >> ammoArray;
        } catch (const std::exception&) {
            throw std::runtime_error("Failed to parse ammo JSON");
        }

        if (!ammoArray.is_array()) {
            throw std::runtime_error("Ammo JSON must be an array");
        }

        // Allocate dynamic array
        m_count = static_cast<int>(ammoArray.size());
        m_capacity = m_count;
        m_entries = new AmmoParameters[m_capacity];

        int idx = 0;
        for (const auto& item : ammoArray) {
            try {

                auto nameStr = item["name"].get<std::string>();
                char name[32];
                std::strncpy(name, nameStr.c_str(), 31);
                name[31] = '\0';

                float mass = item["mass"];
                float drag = item["drag"];
                float lift = item["lift"];

                AmmoType type = (lift > 0.01f) ? AmmoType::Gliding : AmmoType::Standard;

                m_entries[idx] = AmmoParameters(name, mass, drag, lift, type);
                
                DEBUG("Loaded ammo: " << name << " (mass=" << mass << ")");
                idx++;

            } catch (const std::exception&) {
                delete[] m_entries;
                m_entries = nullptr;
                throw std::runtime_error("Invalid ammo entry in JSON");
            }
        }

        LOG("Loaded " << m_count << " ammunition types");
    }

    const AmmoParameters* AmmoDatabase::find(const char* name) const {
        for (int i = 0; i < m_count; ++i) {
            if (std::strcmp(m_entries[i].getName(), name) == 0) {
                return &m_entries[i];
            }
        }
        return nullptr;
    }

}
