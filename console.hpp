#ifndef CONSOLE_H_
#define CONSOLE_H_

#include<SFML/graphics.hpp>

class ConsoleManager {
    public:
        ConsoleManager(sf::Window *Parent);
        ~ConsoleManager(){}

        sf::RenderWindow* getConsoleWindowPtr();

        void render();
    private:
        sf::CircleShape circle;
        sf::RenderWindow ConsoleWindow;
        sf::Window *ParentWindow;
};

#endif