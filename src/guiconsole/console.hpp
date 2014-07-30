#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <TGUI/TGUI.hpp>

class ConsoleManager {
    public:
        ConsoleManager(sf::Window *Parent);
        ConsoleManager();
        ~ConsoleManager(){}

        void init();
        void bindParentWindow(sf::Window *Parent);

        void handleEvent(sf::Event& event,bool& run);
        void handleCommand();

        void render();
    private:
        //technically 12 since we start at zero
        static const int COMMAND_COUNT = 11;
        //console window and pointer to main window
        sf::RenderWindow d_console_window;
        sf::Window *d_parent_window;
        void translateCommandsUp();

        tgui::Gui gui;
        tgui::EditBox::Ptr d_console_edit_box;
        tgui::ChatBox::Ptr d_console_command_list;
};

#endif
