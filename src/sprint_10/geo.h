#pragma once

/*
 * Description: Announces coordinates on the earth's surface and
 * calculates the distance between them
 */

#include <cmath>

namespace geo {

struct Coordinates {
    double lat{0.};
    double lng{0.};
};

double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo