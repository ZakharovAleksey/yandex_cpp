#include "map_renderer.h"

#include "geo.h"
#include "svg.h"

namespace render_settings {

Visualization& Visualization::SetScreen(const Screen& screen) {
    screen_ = screen;
    return *this;
}
Visualization& Visualization::SetLineWidth(double width) {
    line_width_ = width;
    return *this;
}
Visualization& Visualization::SetStopRadius(double radius) {
    stop_radius_ = radius;
    return *this;
}

Visualization& Visualization::SetLabels(LabelType type, Label label) {
    labels_.emplace(type, label);
    return *this;
}

Visualization& Visualization::SetUnderLayer(UnderLayer layer) {
    under_layer_ = std::move(layer);
    return *this;
}

Visualization& Visualization::SetColors(std::vector<svg::Color> colors) {
    colors_ = std::move(colors);
    return *this;
}

}  // namespace render_settings
