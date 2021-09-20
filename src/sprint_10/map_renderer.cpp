#include "map_renderer.h"

// TODO: remove
#include <fstream>

namespace render {

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

/* MAP IMAGE RENDERED */

MapImageRenderer::MapImageRenderer(const catalogue::TransportCatalogue& catalogue, const Visualization& settings)
    : catalogue_(catalogue), settings_(settings) {}

svg::Document MapImageRenderer::GetImage() const {
    return image_;
}

void MapImageRenderer::Render() {
    PutRouteLines();
    PutRouteNames();
    PutStopCircles();
    PutStopNames();
}

void MapImageRenderer::PutRouteLines() {}
void MapImageRenderer::PutRouteNames() {}
void MapImageRenderer::PutStopCircles() {}
void MapImageRenderer::PutStopNames() {}

/* RENDERING METHODS */

void RenderTransportMap(const catalogue::TransportCatalogue& catalogue, const Visualization& settings) {
    MapImageRenderer renderer{catalogue, settings};
    renderer.Render();

    const auto image = renderer.GetImage();

    // TODO: remove - temporary
    std::fstream out("D:\\education\\cpp\\yandex_cpp\\out.svg");
    image.Render(out);
}

}  // namespace render
