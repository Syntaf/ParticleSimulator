#include <SFML/graphics.hpp>
#include <SFML/OpenGL.hpp>
#include "console.hpp"

ConsoleManager::ConsoleManager(sf::Window *Parent):
    ParentWindow(Parent)
{
    ConsoleWindow.create(sf::VideoMode(400,200),
        "Console",
        sf::Style::Default,
        sf::ContextSettings(32, 8, 0, 3, 3)
    );
    ConsoleWindow.setPosition( sf::Vector2i(
        Parent->getPosition().x - Parent->getSize().x/2,
        Parent->getPosition().y));

    circle = sf::CircleShape(15.0f);
    circle.setPosition(0.0f,0.0f);
}

bool ConsoleManager::pollEvent(sf::Event& event)
{
    return ConsoleWindow.pollEvent(event);
}

void ConsoleManager::render()
{
    ConsoleWindow.draw(circle);
    ConsoleWindow.display();
}
