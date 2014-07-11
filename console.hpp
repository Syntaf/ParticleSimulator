#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <SFML/Graphics.hpp>

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
};

#endif
