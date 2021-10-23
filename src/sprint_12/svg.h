#pragma once

#include <any>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace svg {

/* ---------------- TYPES ---------------- */

struct Point {
    Point() = default;
    Point(double x, double y) : x(x), y(y) {}

    double x{0.};
    double y{0.};
};

/*
 * Support structure for SVG output
 */
struct RenderContext {
public:  // Constructors
    explicit RenderContext(std::ostream& out) : out(out) {}

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out), indent_step(indent_step), indent(indent) {}

public:  // Methods
    [[nodiscard]] RenderContext Indented() const;
    void RenderIndent() const;

public:  // Fields
    std::ostream& out;
    int indent_step{0};
    int indent{0};
};

/* ---------------- COLOR TYPES ---------------- */

struct Rgb {
    uint8_t red{0};
    uint8_t green{0};
    uint8_t blue{0};

    Rgb() = default;
    Rgb(uint8_t red, uint8_t green, uint8_t blue) : red(red), green(green), blue(blue) {}
};

struct Rgba : public Rgb {
    double opacity{1.};

    Rgba() = default;
    Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity) : Rgb(red, green, blue), opacity(opacity) {}
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

inline const Color NoneColor{"none"};

/*
 * Used for Color output in std::ostream
 */
struct ColorPrinter {
    std::ostream& os;

    void operator()(std::monostate) const;
    void operator()(const std::string& color) const;
    void operator()(Rgb color) const;
    void operator()(Rgba color) const;
};

std::ostream& operator<<(std::ostream& os, const Color& color);

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

std::ostream& operator<<(std::ostream& os, const StrokeLineCap& value);

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& value);

/* ---------------- PRIMITIVES ---------------- */

/*
 * Abstract parent class Object is used for universe approach of SVG-documents storage
 * Implemented with "Template" programming pattern.
 */
class Object {
public:  // Methods
    void Render(const RenderContext& context) const;
    void Render(std::ostream& os) const;

public:  // Destructor
    virtual ~Object() = default;

private:  // Methods
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
 * Class for storage of additional attributes for objects.
 * Separate class, because not all objects have these attributes (<image>).
 * Approach: Curiously Recurring Template Pattern
 */
template <class ObjectType>
class PathProps {
public:  // Methods
    ObjectType& SetFillColor(Color color) {
        color_ = color;
        return AsObjectType();
    }
    ObjectType& SetStrokeColor(Color stroke_color) {
        stroke_color_ = stroke_color;
        return AsObjectType();
    }
    ObjectType& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsObjectType();
    }
    ObjectType& SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = line_cap;
        return AsObjectType();
    }
    ObjectType& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = line_join;
        return AsObjectType();
    }

protected:  // Destructor
    ~PathProps() = default;

protected:  // Methods
    void RenderAttrs(std::ostream& os) const {
        using namespace std::literals;

        // By default, we assume that primitives declaration has space at the end

        PrintProperty(os, "fill"sv, color_);
        PrintProperty(os, "stroke"sv, stroke_color_);
        PrintProperty(os, "stroke-width"sv, stroke_width_);
        PrintProperty(os, "stroke-linecap"sv, line_cap_);
        PrintProperty(os, "stroke-linejoin"sv, line_join_);
    }

    void RenderAttrs(const RenderContext& context) const {
        RenderContext(context.out);
    }

protected:  // Fields
    std::optional<Color> color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;

private:  // Methods
    ObjectType& AsObjectType() {
        return static_cast<ObjectType&>(*this);
    }

    template <class PropertyType>
    void PrintProperty(std::ostream& os, std::string_view tag_name,
                       const std::optional<PropertyType>& tag_value) const {
        if (tag_value)
            os << " " << tag_name << "=\"" << *tag_value << "\"";
    }
};

/*
 * Circle represent <circle> tag
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:  // Methods
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:  // Methods
    void RenderObject(const RenderContext& context) const override;

private:  // Fields
    Point center_;
    double radius_ = 1.0;
};

/*
 * Polyline represent <polyline> tag
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:  // Methods
    Polyline& AddPoint(Point point);

private:  // Methods
    void RenderObject(const RenderContext& context) const override;

private:  // Fields
    std::vector<Point> vertexes_;
};

/*
 * Text represent <text> tag
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:  // Types
    struct EscapeCharacter {
        char character;
        std::string replacement;
    };

public:  // Methods
    Text& SetPosition(Point position = Point());

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset = Point());

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:  // Methods
    void RenderObject(const RenderContext& context) const override;

    //! Replace all SVG escape characters for the input string
    [[nodiscard]] std::string PreprocessTest(const std::string& input_text) const;

private:  // Constants
    //! Sequence of characters should start with '&' because otherwise
    //! all next characters (&quot, ...) will be with replaced '&'
    inline static const std::vector<EscapeCharacter> kEscapeCharacters{
        {'&', "&amp;"}, {'"', "&quot;"}, {'\'', "&apos;"}, {'<', "&lt;"}, {'>', "&gt;"}};

private:  // Fields
    Point position_;
    Point offset_;

    uint32_t font_size_{1};
    std::string font_family_;
    std::string font_weight_;

    std::string text_;
};

/* ---------------- OBJECT CONTAINERS ---------------- */

class ObjectContainer {
public:  // Destructor
    virtual ~ObjectContainer() = default;

public:  // Methods
    virtual void AddPtr(std::unique_ptr<Object>&& /* objects */) = 0;

    template <typename ObjectType>
    void Add(ObjectType obj) {
        AddPtr(std::make_unique<ObjectType>(std::move(obj)));
    }

protected:  // Fields
    std::vector<std::unique_ptr<Object>> storage_;
};

class Document final : public ObjectContainer {
public:  // Methods
    void AddPtr(std::unique_ptr<Object>&& object) override;
    void Render(std::ostream& out) const;
};

/* ---------------- FIGURES ---------------- */

class Drawable {
public:  // Destructor
    virtual ~Drawable() = default;

public:  // Methods
    virtual void Draw(svg::ObjectContainer& /* container */) const = 0;
};

}  // namespace svg

namespace shapes {

class Star : public svg::Drawable {
public:  // Constructor
    Star(svg::Point center, double outer_radius, double inner_radius, int rays_count)
        : center_(center), outer_radius_(outer_radius), inner_radius_(inner_radius), rays_count_(rays_count) {}

public:  // Methods
    void Draw(svg::ObjectContainer& container) const override;

private:  // Fields
    svg::Point center_;
    double outer_radius_{0.};
    double inner_radius_{0.};
    int rays_count_{0};

    svg::Color fill_color_{"red"};
    svg::Color stroke_color_{"black"};
};

class Snowman : public svg::Drawable {
public:  // Constructors
    Snowman(svg::Point head_center, double head_radius) : head_center_(head_center), head_radius_(head_radius) {}

public:  // Methods
    void Draw(svg::ObjectContainer& container) const override;

private:  // Fields
    svg::Point head_center_;
    double head_radius_;

    svg::Color fill_color_{"rgb(240,240,240)"};
    svg::Color stroke_color_{"black"};
};

class Triangle : public svg::Drawable {
public:  // Constructor
    Triangle(svg::Point first, svg::Point second, svg::Point third) : first_(first), second_(second), third_(third) {}

public:  // Methods
    void Draw(svg::ObjectContainer& container) const override;

private:  // Fields
    svg::Point first_;
    svg::Point second_;
    svg::Point third_;
};

}  // namespace shapes
