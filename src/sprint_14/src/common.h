#pragma once

/*
 * Details: common tools used in a huge bunch of files
 */

#include <string>
#include <vector>

struct Size {
    int width = 0;
    int height = 0;
};

struct Point {
    int x = 0;
    int y = 0;
};

/// @brief represents image, with pixels as symbols
/// @details First index (on std::vector) - image rows, Y-coordinate
/// @details Second index (on std::string) - image columns, X-coordinate
/// @details Assume that length of all rows are equal
using Image = std::vector<std::string>;

inline Size GetImageSize(const Image& image) {
    const int width = image.empty() ? 0 : static_cast<int>(image[0].size());
    const int height = static_cast<int>(image.size());
    return {width, height};
}

inline bool IsPointInEllipse(Point p, Size size) {
    double x = (p.x + 0.5) / (size.width / 2.0) - 1;
    double y = (p.y + 0.5) / (size.height / 2.0) - 1;

    return x * x + y * y <= 1;
}