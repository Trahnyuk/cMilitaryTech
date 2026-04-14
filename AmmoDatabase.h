#include "AmmoParameters.h"

#include <string>
#include <vector>
#include <optional>

namespace cMiltech {

    class AmmoDatabase {
    public:
        
        AmmoDatabase();

       
        std::optional<AmmoParameters> find(const std::string& name) const;

      
        std::size_t count() const noexcept;
        const std::vector<AmmoParameters>& all() const noexcept;

    private:
        std::vector<AmmoParameters> m_entries;
    };

}
