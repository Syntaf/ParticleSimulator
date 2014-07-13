#ifdef USE_TGUI
#include <TGUI/TGUI.hpp>
#else
#include <SFML/Graphics.hpp>
#endif
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
#ifdef USE_TGUI
    gui.setWindow(ConsoleWindow);
    gui.setGlobalFont("TGUI/fonts/DejaVuSans.ttf");
#endif
    //to line console up with main window, divide parent size by some constant(2.35)
    ConsoleWindow.setPosition( sf::Vector2i(
        (int)(Parent->getPosition().x - Parent->getSize().x/2.35),
        Parent->getPosition().y));

}


void ConsoleManager::handleEvent(sf::Event& event, bool& run)
{
    while(ConsoleWindow.pollEvent(event))
    {
        if(event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
            run = false;
#ifdef USE_TGUI
        //pass event to all widgets
        gui.handleEvent(event);
#endif
    }
}

void ConsoleManager::render()
{
    ConsoleWindow.clear();

    ConsoleWindow.pushGLStates();
#ifdef USE_TGUI
    gui.draw();
#endif
    ConsoleWindow.display();
    ConsoleWindow.popGLStates();
}
