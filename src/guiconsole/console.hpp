#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <TGUI/TGUI.hpp>
#include "../particlemanager.hpp"

class ConsoleManager {
    public:
        // Simple constructor to create and set the position of the
        //	 new window. Lines up with the parent window.
        ConsoleManager(sf::Window *parent);
        ConsoleManager();
        ~ConsoleManager(){}

        // Initialize console window and create all necessary TGUI
        //	 widgets to best simulate a console
        void init();

        // Bind class pointer to particle manager passed as argument.
        //	 particle manager will be used to get and set values of each
        //	 particle according to the parsed input received. 
        void bindParticleManager(ParticleManager *Particlemanager);

        // Handle window events passed in from app's run() function. Deals
        //	 with events such as resizing and escape.
        void handleEvent(sf::Event& event,bool& run);

        // Base function for handling input sent from TGUI widget when user
        //	 enters text into the console. Calls the proper set command below
        //	 according to the first word( get set help etc.. etc..)
        void handleCommand();

        // Function providing shortcut to print text to the console in a
        //	 'console esqueue' feel. Newlines are denoted as the '%' symbol
        //	 in the passed string. So "Error%bad input" would be printed as
        //	 "Error\nbad input"
        void printToConsole(const std::string& text);

        // Render TGUI widgets and window to screen, ensures to push GL states
        //	 before rendering to ensure it will not skew with the main window
        //	 and the particles displaying.
        void render();
    private:

        static const int COMMAND_COUNT = 16;            // number of commands we support

        sf::RenderWindow d_console_window;              // our window for displaying console
        sf::Window *d_parent_window;                    // pointer to the parent window
        ParticleManager *d_particle_manager;            // pointer to the particle manager

        void translateCommandsUp();                     // moves all commands up in the console
        void handleSetCommand(const std::string& str);  // handles set commands
        void handleGetCommand(const std::string& str);  // handles get commands
        void handleHelpCommand(const std::string& str); // handles help commands

        tgui::Gui gui;                                  // our gui for displaying TGUI widgets
        tgui::EditBox::Ptr d_console_edit_box;          // edit box for input
        tgui::ChatBox::Ptr d_console_command_list;      // displays past commands
};

#endif
