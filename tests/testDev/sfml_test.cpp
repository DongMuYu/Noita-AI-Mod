#include <SFML/Graphics.hpp>
#include "Square.h"
#include "Triangle.h"
#include "Circle.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(600, 200), "SFML Shapes");
    
    // 创建圆形
    Circle circle(50.f, 250, 50, sf::Color::Green);
    
    // 创建正方形
    Square square(100.f, {50, 50}, sf::Color::Blue);
    
    // 创建三角形
    Triangle triangle(100.f, {450, 50}, sf::Color::Red);
    
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        window.clear();
        window.draw(square);
        window.draw(circle);
        window.draw(triangle);
        window.display();
    }
    
    return 0;
}