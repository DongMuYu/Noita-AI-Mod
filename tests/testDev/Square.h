#ifndef SQUARE_H
#define SQUARE_H

#include <SFML/Graphics.hpp>

class Square : public sf::Drawable {
private:
    sf::RectangleShape shape;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

public:
    Square(float size, sf::Vector2f position, sf::Color color);
    void setPosition(sf::Vector2f position);
    void setColor(sf::Color color);
};

#endif // SQUARE_H