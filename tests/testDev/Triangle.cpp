#include "Triangle.h"

Triangle::Triangle(float size, sf::Vector2f position, sf::Color color) {
    shape.setPointCount(3);
    shape.setPoint(0, sf::Vector2f(size/2, 0));
    shape.setPoint(1, sf::Vector2f(0, size));
    shape.setPoint(2, sf::Vector2f(size, size));
    shape.setPosition(position);
    shape.setFillColor(color);
}

void Triangle::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(shape, states);
}

void Triangle::setPosition(sf::Vector2f position) {
    shape.setPosition(position);
}

void Triangle::setColor(sf::Color color) {
    shape.setFillColor(color);
}