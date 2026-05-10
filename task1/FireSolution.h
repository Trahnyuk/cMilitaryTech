#include <optional>
#include <string>

namespace cMiltech {

    struct Point2D {
        float x;
        float y;
    };

    class FireSolution {
    public:
        FireSolution(Point2D firePoint,
                     std::optional<Point2D> intermediatePoint,
                     float timeOfFlight,
                     float horizontalDistance);

        const Point2D&                  getFirePoint()          const noexcept;
        const std::optional<Point2D>&   getIntermediatePoint()  const noexcept;
        float                           getTimeOfFlight()       const noexcept;
        float                           getHorizontalDistance() const noexcept;
        bool                            hasManeuver()           const noexcept;

     
        void writeToFile(const std::string& filePath) const;

        void print() const;

    private:
        Point2D                 m_firePoint;
        std::optional<Point2D>  m_intermediatePoint;
        float                   m_timeOfFlight;
        float                   m_horizontalDistance;
    };

} 
