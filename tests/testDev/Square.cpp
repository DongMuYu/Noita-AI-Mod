#include "Square.h"

Square::Square(float size, sf::Vector2f position, sf::Color color) {
    shape.setSize(sf::Vector2f(size, size));
    shape.setPosition(position);
    shape.setFillColor(color);
}

void Square::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(shape, states);
}

void Square::setPosition(sf::Vector2f position) {
    shape.setPosition(position);
}

void Square::setColor(sf::Color color) {
    shape.setFillColor(color);
}