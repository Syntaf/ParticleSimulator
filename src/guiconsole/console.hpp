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

        void init();

        void handleEvent(sf::Event& event,bool& run);

        void render();
    private:
        static const int COMMAND_COUNT = 12;
        sf::CircleShape circle;
        sf::RenderWindow ConsoleWindow;
        sf::Window *ParentWindow;
#ifdef USE_TGUI
        void translateCommandsUp();
        tgui::Gui gui;
        tgui::EditBox::Ptr ConsoleEditBox;
        tgui::ListBox::Ptr ConsoleCommandList;
#endif
};

#endif
