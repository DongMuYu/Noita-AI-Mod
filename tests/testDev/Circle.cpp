#include "Circle.h"

Circle::Circle(float radius, float x, float y, sf::Color color) {
    shape.setRadius(radius);
    shape.setPosition(x, y);
    shape.setFillColor(color);
}

void Circle::setPosition(float x, float y) {
    shape.setPosition(x, y);
}

void Circle::setColor(sf::Color color) {
    shape.setFillColor(color);
}

sf::FloatRect Circle::getBounds() const {
    return shape.getGlobalBounds();
}

void Circle::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(shape, states);
}