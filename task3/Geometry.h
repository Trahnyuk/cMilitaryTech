#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cmath>

namespace cMiltech {

    struct Point2D {
        float x;
        float y;

        // Operator overloading
        Point2D operator+(const Point2D& other) const {
            return {x + other.x, y + other.y};
        }

        Point2D operator-(const Point2D& other) const {
            return {x - other.x, y - other.y};
        }

        Point2D operator*(float scalar) const {
            return {x * scalar, y * scalar};
        }

        Point2D operator/(float scalar) const {
            return {x / scalar, y / scalar};
        }

        bool operator==(const Point2D& other) const {
            const float epsilon = 1e-6f;
            return std::fabs(x - other.x) < epsilon && 
                   std::fabs(y - other.y) < epsilon;
        }

        // Helper methods
        float length() const {
            return std::hypot(x, y);
        }

        Point2D normalized() const {
            float len = length();
            if (len < 1e-6f) {
                return {0.0f, 0.0f};
            }
            return *this / len;
        }
    };

    // Free functions for convenience
    inline float length(const Point2D& p) {
        return p.length();
    }

    inline Point2D normalize(const Point2D& p) {
        return p.normalized();
    }

    // Scalar * Point2D (allows: 5.0f * point)
    inline Point2D operator*(float scalar, const Point2D& p) {
        return p * scalar;
    }

}

#endif
