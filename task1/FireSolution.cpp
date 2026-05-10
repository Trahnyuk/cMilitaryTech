#include "FireSolution.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>

using std::string;
using std::ofstream;
using std::cout;
using std::fixed;
using std::setprecision;
using std::runtime_error;

namespace cMiltech {

    FireSolution::FireSolution(Point2D firePoint,
                               std::optional<Point2D> intermediatePoint,
                               float timeOfFlight,
                               float horizontalDistance)
        : m_firePoint(firePoint)
        , m_intermediatePoint(intermediatePoint)
        , m_timeOfFlight(timeOfFlight)
        , m_horizontalDistance(horizontalDistance)
    {}

    const Point2D& FireSolution::getFirePoint() const noexcept {
        return m_firePoint;
    }

    const std::optional<Point2D>& FireSolution::getIntermediatePoint() const noexcept {
        return m_intermediatePoint;
    }

    float FireSolution::getTimeOfFlight()       const noexcept { return m_timeOfFlight; }
    float FireSolution::getHorizontalDistance() const noexcept { return m_horizontalDistance; }
    bool  FireSolution::hasManeuver()           const noexcept { return m_intermediatePoint.has_value(); }

    void FireSolution::writeToFile(const string& filePath) const {
        ofstream out(filePath);
        if (!out.is_open()) {
            throw runtime_error("Could not open output file: " + filePath);
        }

        out << fixed << setprecision(3);

        if (m_intermediatePoint.has_value()) {
            out << m_intermediatePoint->x << " "
                << m_intermediatePoint->y << " ";
        }

        out << m_firePoint.x << " " << m_firePoint.y << '\n';
    }

    void FireSolution::print() const {
        cout << fixed << setprecision(3);
        cout << "Time of flight:       " << m_timeOfFlight       << " s\n"
             << "Horizontal distance:  " << m_horizontalDistance << " m\n";

        if (m_intermediatePoint.has_value()) {
            cout << "Intermediate point:   ("
                 << m_intermediatePoint->x << ", "
                 << m_intermediatePoint->y << ")\n";
        }

        cout << "Fire point:           ("
             << m_firePoint.x << ", "
             << m_firePoint.y << ")\n";
    }

}
