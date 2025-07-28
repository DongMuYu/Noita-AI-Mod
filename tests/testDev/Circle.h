#ifndef CIRCLE_H
#define CIRCLE_H

#include <SFML/Graphics.hpp>

class Circle : public sf::Drawable {
private:
    sf::CircleShape shape;
public:
    Circle(float radius, float x, float y, sf::Color color);
    void setPosition(float x, float y);
    void setColor(sf::Color color);
    sf::FloatRect getBounds() const;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

#endif