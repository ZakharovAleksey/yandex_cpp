#pragma once

#include "svg.h"
#include "transport_catalogue.h"

namespace render {

enum class LabelType { Bus = 0, Stop = 1 };

// Used to project coordinates on map
struct Screen {
    double width_{0.};
    double height_{0.};
    double padding_{0.};
};

// Background color under the names of stops and routes
struct UnderLayer {
    svg::Color color_;
    int width_;
};

// Settings to display stops and buses names
struct Label {
    int font_size_{0};
    svg::Point offset_;
};

/*
 * Class stores all visualization settings for map image3 rendering
 */

class Visualization {
public:
    Visualization() = default;

    Visualization& SetScreen(const Screen& screen);
    Visualization& SetLineWidth(double width);
    Visualization& SetStopRadius(double radius);

    Visualization& SetLabels(LabelType type, Label label);
    Visualization& SetUnderLayer(UnderLayer layer);
    Visualization& SetColors(std::vector<svg::Color> colors);

private:
    Screen screen_;
    double line_width_{0.};
    double stop_radius_{0.};

    std::unordered_map<LabelType, Label> labels_;
    UnderLayer under_layer_;
    std::vector<svg::Color> colors_;
};

/* MAP IMAGE RENDERED */

/*
 * The map consists of four types of objects.
 * The order of their output:
 * - route lines
 * - route names
 * - circles indicating stops
 * - stop names
 */

class MapImageRenderer {
public:  // Constructor
    MapImageRenderer(const catalogue::TransportCatalogue& catalogue, const Visualization& settings);

public:  // Method
    svg::Document GetImage() const;
    void Render();

private:  // Method
    void PutRouteLines();
    void PutRouteNames();
    void PutStopCircles();
    void PutStopNames();



private:  // Fields
    const catalogue::TransportCatalogue& catalogue_;
    const Visualization& settings_;
    svg::Document image_;
};

/* RENDERING METHODS */

void RenderTransportMap(const catalogue::TransportCatalogue& catalogue, const Visualization& settings);

}  // namespace render
