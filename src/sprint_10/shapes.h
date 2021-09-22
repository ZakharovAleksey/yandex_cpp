#pragma once
#include <memory>

#include "texture.h"

// Поддерживаемые виды фигур: прямоугольник и эллипс
enum class ShapeType { RECTANGLE, ELLIPSE };

class Shape {
public:  // Constructor
    explicit Shape(ShapeType type) : type_(type) {}

public:  // Methods
    void SetPosition(Point pos) {
        position_ = pos;
    }

    void SetSize(Size size) {
        size_ = size;
    }

    void SetTexture(std::shared_ptr<Texture> texture) {
        texture_ = std::move(texture);
    }

    /*
     * Method draws a shape on the specified image
     * Depending on the type of shape, either an ellipse or a rectangle should be drawn
     * The pixels of the shape that go outside the texture, and also in the case
     * when the texture is not specified, should be rendered with a dot '.'
     * The parts of the shape outside the bounds of the image object should be discarded.
     */
    void Draw(Image& image) const {
        const auto [image_width, image_height] = GetImageSize(image);

        int x_global{0};
        int y_global{0};
        char pixel;

        for (int y = 0; y < size_.height; ++y) {
            for (int x = 0; x < size_.width; ++x) {
                y_global = position_.y + y;
                x_global = position_.x + x;

                if (y_global < 0 || y_global >= image_height || x_global < 0 || x_global >= image_width)
                    continue;

                pixel = (!texture_) ? '.' : texture_->GetPixelColor({x, y});

                if ((type_ == ShapeType::ELLIPSE && IsPointInEllipse({x, y}, size_)) || (type_ == ShapeType::RECTANGLE))
                    image[y_global][x_global] = pixel;
            }
        }
    }

private:  // Fields
    ShapeType type_;
    Point position_;
    Size size_;
    std::shared_ptr<Texture> texture_;
};