#ifndef CONSOLE_H_
#define CONSOLE_H_

#ifdef USE_TGUI
#include <TGUI/TGUI.hpp>
#else
#include <SFML\Graphics.hpp>
#endif

class ConsoleManager {
    public:
        ConsoleManager(sf::Window *Parent);
        ~ConsoleManager(){}

        void handleEvent(sf::Event& event,bool& run);

        void render();
    private:
        sf::CircleShape circle;
        sf::RenderWindow ConsoleWindow;
        sf::Window *ParentWindow;
#ifdef USE_TGUI
        tgui::Gui gui;
#endif
};

#endif
