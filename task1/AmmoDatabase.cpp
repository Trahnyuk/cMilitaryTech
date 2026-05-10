#include "AmmoDatabase.h"

#include <algorithm>

using std::string;
using std::vector;
using std::optional;
using std::nullopt;

namespace cMiltech {

    AmmoDatabase::AmmoDatabase() {
       
        // AmmoParameters(name, mass, drag, lift, AmmoType)
        m_entries = {
            { "VOG-17",      0.35f, 0.07f, 0.0f, AmmoType::Standard },
            { "M67",         0.60f, 0.10f, 0.0f, AmmoType::Standard },
            { "RKG-3",       1.20f, 0.10f, 0.0f, AmmoType::Standard },
            { "GLIDING-VOG", 0.45f, 0.10f, 1.0f, AmmoType::Gliding  },
            { "GLIDING-RKG", 1.40f, 0.10f, 1.0f, AmmoType::Gliding  },
        };
    }

    optional<AmmoParameters> AmmoDatabase::find(const string& name) const {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
            [&name](const AmmoParameters& a) {
                return a.getName() == name;
            });

        if (it == m_entries.end()) {
            return nullopt;
        }
        return *it;
    }

    std::size_t AmmoDatabase::count() const noexcept {
        return m_entries.size();
    }

    const vector<AmmoParameters>& AmmoDatabase::all() const noexcept {
        return m_entries;
    }

}
