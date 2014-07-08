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
    
    //to line console up with main window, divide parent size by some constant(2.35)
    ConsoleWindow.setPosition( sf::Vector2i(
        Parent->getPosition().x - Parent->getSize().x/2.35,
        Parent->getPosition().y));

    circle = sf::CircleShape(15.0f);
    circle.setPosition(0.0f,0.0f);
}

sf::RenderWindow* ConsoleManager::getConsoleWindowPtr()
{
    return &ConsoleWindow;
}

void ConsoleManager::render()
{
    ConsoleWindow.pushGLStates();
    ConsoleWindow.draw(circle);
    ConsoleWindow.display();
    ConsoleWindow.popGLStates();
}
