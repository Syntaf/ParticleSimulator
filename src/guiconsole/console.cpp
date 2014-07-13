#ifdef USE_TGUI
#include <TGUI/TGUI.hpp>
#else
#include <SFML/Graphics.hpp>
#endif
#include <SFML/OpenGL.hpp>
#include "console.hpp"
#include <cstdlib>

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

void ConsoleManager::init()
{
    ConsoleEditBox = tgui::EditBox::Ptr(gui);
    ConsoleEditBox->load("TGUI/widgets/black.conf");
    ConsoleEditBox->setSize(400,20);
    ConsoleEditBox->setPosition(0,180);
    ConsoleEditBox->setTextSize(13);
    ConsoleEditBox->setMaximumCharacters(32);
    ConsoleEditBox->setText("> ");
    

    ConsoleCommandList = tgui::ListBox::Ptr(gui);
    ConsoleCommandList->load("TGUI/widgets/black.conf");
    ConsoleCommandList->setSize(400,180);
    ConsoleCommandList->setPosition(0,0);
    ConsoleCommandList->removeScrollbar();
    ConsoleCommandList->setMaximumItems(COMMAND_COUNT+1);
    ConsoleCommandList->setItemHeight(15);
    for(int i = 0; i < COMMAND_COUNT; i++) {
        ConsoleCommandList->addItem(" ", i);
    }

    //set input box in focus so user does not have to click
    ConsoleEditBox->focus();
}

void ConsoleManager::handleEvent(sf::Event& event, bool& run)
{
    while(ConsoleWindow.pollEvent(event))
    {
        if(event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
            run = false;
        if(event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Return)
            translateCommandsUp();
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

void ConsoleManager::translateCommandsUp()
{
    for(int i = 0; i < 11 ; i++) {
        std::string next = (ConsoleCommandList->getItem(i+1)).toAnsiString();
        ConsoleCommandList->changeItem(i,
            next);
    }
    ConsoleCommandList->changeItem(11, ConsoleEditBox->getText().getData());
    ConsoleEditBox->setText("> ");
}
