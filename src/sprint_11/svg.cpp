#define _USE_MATH_DEFINES
#include "svg.h"

#include <cmath>
#include <sstream>
#include <string_view>

namespace svg {
using namespace std::literals;

void ColorPrinter::operator()(std::monostate) const {
    os << NoneColor;
}

void ColorPrinter::operator()(const std::string& color) const {
    os << color;
}

void ColorPrinter::operator()(Rgb color) const {
    // clang-format off
    os << "rgb("s
       << std::to_string(color.red) << ","s
       << std::to_string(color.green) << ","
       << std::to_string(color.blue) << ")"s;
    // clang-format on
}

void ColorPrinter::operator()(Rgba color) const {
    // clang-format off
    os << "rgba("s
       << std::to_string(color.red) << ","s
       << std::to_string(color.green) << ","s
       << std::to_string(color.blue) << ","s;
    // clang-format on

    os << color.opacity << ")"s;
}

std::ostream& operator<<(std::ostream& os, const Color& color) {
    std::visit(ColorPrinter{os}, color);
    return os;
}

std::ostream& operator<<(std::ostream& os, const StrokeLineCap& value) {
    switch (value) {
        case StrokeLineCap::BUTT:
            return os << "butt"s;
        case StrokeLineCap::ROUND:
            return os << "round"s;
        case StrokeLineCap::SQUARE:
            return os << "square"s;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& value) {
    switch (value) {
        case StrokeLineJoin::ARCS:
            return os << "arcs"s;
        case StrokeLineJoin::BEVEL:
            return os << "bevel"s;
        case StrokeLineJoin::MITER:
            return os << "miter"s;
        case StrokeLineJoin::MITER_CLIP:
            return os << "miter-clip"s;
        case StrokeLineJoin::ROUND:
            return os << "round"s;
    }
    return os;
}

/* ---------------- PRIMITIVES ---------------- */

RenderContext RenderContext::Indented() const {
    return {out, indent_step, indent + indent_step};
}

void RenderContext::RenderIndent() const {
    for (int i = 0; i < indent; ++i)
        out.put(' ');
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    RenderObject(context);

    context.out << std::endl;
}

void Object::Render(std::ostream& os) const {
    RenderContext context(os, 2, 2);
    Render(context);
}

/* ---------------- CIRCLE ---------------- */

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

/* ---------------- POLYLINE ---------------- */

Polyline& Polyline::AddPoint(Point point) {
    vertexes_.emplace_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\"";

    int id{0};
    for (const auto& vertex : vertexes_) {
        if (id++ != 0)
            out << " ";
        out << vertex.x << ","sv << vertex.y;
    }

    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

/* ---------------- TEST ---------------- */

Text& Text::SetPosition(Point position) {
    position_ = position;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    text_ = std::move(data);
    return *this;
}

std::string Text::PreprocessTest(const std::string& input_text) const {
    std::string result = input_text;

    for (const auto& pair : kEscapeCharacters) {
        size_t position{0};
        while (true) {
            position = result.find(pair.character, position);
            if (position == std::string::npos)
                break;
            result.replace(position, 1, pair.replacement);
            position = position + pair.replacement.size();
        }
    }

    return result;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;

    RenderAttrs(out);
    out << " ";
    // Text position
    out << "x=\""sv << position_.x << "\" y=\"" << position_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" dy=\"" << offset_.y << "\""sv;

    // Styling attributes
    out << " font-size=\"" << font_size_ << "\"";
    if (!font_family_.empty())
        out << " font-family=\"" << font_family_ << "\"";
    if (!font_weight_.empty())
        out << " font-weight=\"" << font_weight_ << "\"";

    out << ">" << PreprocessTest(text_) << "</text>";
}

/* ---------------- OBJECT CONTAINER ---------------- */

/* ---------------- DOCUMENT ---------------- */

void Document::AddPtr(std::unique_ptr<Object>&& object) {
    storage_.emplace_back(std::move(object));
}

void Document::Render(std::ostream& out) const {
    // RenderContext context(out, 2, 2);

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (const auto& object : storage_)
        object->Render(out);
    out << "</svg>"sv;
}

}  // namespace svg

namespace shapes {

void Star::Draw(svg::ObjectContainer& container) const {
    svg::Polyline polyline;

    for (int ray_id = 0; ray_id <= rays_count_; ++ray_id) {
        double angle = 2 * M_PI * (ray_id % rays_count_) / rays_count_;
        polyline.AddPoint({center_.x + outer_radius_ * sin(angle), center_.y - outer_radius_ * cos(angle)});

        if (ray_id == rays_count_)
            break;

        angle += M_PI / rays_count_;
        polyline.AddPoint({center_.x + inner_radius_ * sin(angle), center_.y - inner_radius_ * cos(angle)});
    }

    container.Add(polyline.SetFillColor(fill_color_).SetStrokeColor(stroke_color_));
}

void Snowman::Draw(svg::ObjectContainer& container) const {
    // Top circle
    svg::Point current_center{head_center_.x, head_center_.y};
    double current_radius{head_radius_};
    auto top = svg::Circle().SetCenter(current_center).SetRadius(current_radius);

    // Middle circle
    current_center.y += 2. * head_radius_;
    current_radius = 1.5 * head_radius_;
    auto middle = svg::Circle().SetCenter(current_center).SetRadius(current_radius);

    // Bottom circle
    current_center.y += 3. * head_radius_;
    current_radius = 2. * head_radius_;
    auto bottom = svg::Circle().SetCenter(current_center).SetRadius(current_radius);

    for (auto circle : {bottom, middle, top})
        container.Add(std::move(circle).SetFillColor(fill_color_).SetStrokeColor(stroke_color_));
}

void Triangle::Draw(svg::ObjectContainer& container) const {
    container.Add(svg::Polyline().AddPoint(first_).AddPoint(second_).AddPoint(third_).AddPoint(first_));
}
}  // namespace shapes