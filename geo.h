#pragma once
#include <cmath>
constexpr int EARTH_RADIUS = 6371000;

struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }

    static double ComputeDistance(Coordinates from, Coordinates to);
};
