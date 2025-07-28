#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <SFML/Graphics.hpp>

class Triangle : public sf::Drawable {
private:
    sf::ConvexShape shape;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

public:
    Triangle(float size, sf::Vector2f position, sf::Color color);
    void setPosition(sf::Vector2f position);
    void setColor(sf::Color color);
};

#endif // TRIANGLE_H