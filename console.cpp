#include <SFML/Graphics.hpp>
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
        (int)(Parent->getPosition().x - Parent->getSize().x/2.35),
        Parent->getPosition().y));

    circle = sf::CircleShape(15.0f);
    circle.setPosition(0.0f,0.0f);
}
void ConsoleManager::handleEvent(sf::Event& event, bool& run)
{
    while(ConsoleWindow.pollEvent(event)) 
    {
        if(event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
				run = false;
    }
}

void ConsoleManager::render()
{
    ConsoleWindow.pushGLStates();
    ConsoleWindow.draw(circle);
    ConsoleWindow.display();
    ConsoleWindow.popGLStates();
}
