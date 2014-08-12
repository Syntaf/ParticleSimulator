#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <TGUI/TGUI.hpp>
#include "../particlemanager.hpp"

class ConsoleManager {
    public:
        ConsoleManager(sf::Window *Parent);
        ConsoleManager();
        ~ConsoleManager(){}

        //initialization functions
        void init();
        void bindParentWindow(sf::Window *Parent);
        void bindParticleManager(ParticleManager *Particlemanager);

        //handle any events coming into the console window
        //  also handle commands inputed into console
        void handleEvent(sf::Event& event,bool& run);
        void handleCommand();

        //print any input to our console, new lines are
        //  denoted as '%' characters as opposed to \n in
        //  the standard output stream.
        void printToConsole(const std::string& text);

        void render();
    private:
        //technically 12 since we start at zero
        static const int COMMAND_COUNT = 12;
        //console window and pointer to main window
        sf::RenderWindow d_console_window;
        sf::Window *d_parent_window;
        ParticleManager *d_particle_manager;

        void translateCommandsUp();
        void handleSetCommand(const std::string& str);
        void handleGetCommand(const std::string& str);

        tgui::Gui gui;
        tgui::EditBox::Ptr d_console_edit_box;
        tgui::ChatBox::Ptr d_console_command_list;
};

#endif
