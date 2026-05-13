#ifndef AMMODATABASE_H
#define AMMODATABASE_H

#include "AmmoParameters.h"

namespace cMiltech {

    class AmmoDatabase {
    public:
        explicit AmmoDatabase(const char* ammoJsonPath);
        
        ~AmmoDatabase();
        AmmoDatabase(const AmmoDatabase& other);
        AmmoDatabase& operator=(const AmmoDatabase& other);

        const AmmoParameters* find(const char* name) const;

        // Database info
        int count() const noexcept { return m_count; }
        const AmmoParameters* all() const noexcept { return m_entries; }

    private:
        void loadFromJson(const char* ammoJsonPath);
        void cleanup();
        void copyFrom(const AmmoDatabase& other);

        AmmoParameters* m_entries;  
        int m_count;
        int m_capacity;
    };

}

#endif
