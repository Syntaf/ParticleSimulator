#include <TGUI/TGUI.hpp>
#include "../app.hpp"
#include <SFML/OpenGL.hpp>
#include "console.hpp"
#include "consolecommands.hpp"
#include <cstdlib>
#include <sstream>
#include <algorithm>

ConsoleManager::ConsoleManager(sf::Window *Parent):
    d_parent_window(Parent)
{
    d_console_window.create(sf::VideoMode(400,200),
        "Console",
        sf::Style::Default,
        sf::ContextSettings(32, 8, 0, 3, 3)
    );

    gui.setWindow(d_console_window);
    gui.setGlobalFont("TGUI/fonts/DejaVuSans.ttf");

    //to line console up with main window, divide parent size by some constant(2.35)
    d_console_window.setPosition( sf::Vector2i(
        (int)(Parent->getPosition().x - Parent->getSize().x/2.35),
        Parent->getPosition().y));

}

void ConsoleManager::bindParentWindow(sf::Window *Parent)
{
    d_parent_window = Parent;
}

void ConsoleManager::bindParticleManager(ParticleManager *Particlemanager) 
{
    d_particle_manager = Particlemanager;
}

void ConsoleManager::init()
{
    //here we initialize the gui objects. now TGUI, or any other
    //  SFML gui for that matter does offer any quick solutions to
    //  creating a console, so here we are making a very ugly console
    //  that will make the user think it is a fully functional window.
    //  we place an edit box at the bottom, and simply use a chat box
    //  to display previous commands, everything blends so the user will
    //  think this is all one window!
    d_console_edit_box = tgui::EditBox::Ptr(gui);
    d_console_edit_box->load("TGUI/widgets/black.conf");
    d_console_edit_box->setSize(400,20);
    d_console_edit_box->setPosition(0,180);
    d_console_edit_box->setTextSize(12);
    d_console_edit_box->setMaximumCharacters(32);
    d_console_edit_box->setText("> ");
    

    d_console_command_list = tgui::ChatBox::Ptr(gui);
    d_console_command_list->load("TGUI/widgets/black.conf");
    d_console_command_list->setSize(400,180);
    d_console_command_list->setPosition(0,0);
    d_console_command_list->removeScrollbar();
    d_console_command_list->setLinesStartFromBottom(true);
    d_console_command_list->setLineLimit(COMMAND_COUNT+1);
    d_console_command_list->setTextSize(12);
    d_console_command_list->setTextColor(d_console_edit_box->getTextColor());

    //set input box in focus so user does not have to click
    d_console_edit_box->focus();
}

void ConsoleManager::handleEvent(sf::Event& event, bool& run)
{
    while(d_console_window.pollEvent(event))
    {
        if(event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
            run = false;
        if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return) {
            handleCommand();
        }
        //pass event to all widgets
        gui.handleEvent(event);
    }
}

void ConsoleManager::handleCommand()
{
    //get current text inside console
    std::string command = d_console_edit_box->getText().toAnsiString();

    //determine distance from '>' character to end of first command
    std::size_t dist_command_sub_string = 
        std::distance(
            command.begin()+2, 
            std::find(command.begin()+2, command.end(), ' ')
        );

    //get substring holding just the command
    std::string command_sub_string = command.substr(
        2, 
        dist_command_sub_string
    );
    
    //if there is more after the command, set command to JUST that
    if(dist_command_sub_string+2 != command.length()) {
        command = command.substr(dist_command_sub_string+3,
            std::distance(command.begin()+dist_command_sub_string+3, command.end())
            );
    }

    consolecommands::Key command_key;

    if(!consolecommands::isValidCommandKey(command_sub_string, command_key)) {
        d_console_command_list->
            addLine(d_console_edit_box->getText().getData());
        printToConsole("Invalid Command");
        d_console_edit_box->setText("> ");
    }else{
        switch(command_key) {
            case consolecommands::GET:
                break;
            case consolecommands::SET:
                handleSetCommand(command);
                break;
            case consolecommands::EXIT:
                App::procClose();
                break;
            case consolecommands::HELP:
                printToConsole("List of available commands:%GET <var>%SET <var>%EXIT");
                break;
        }
        translateCommandsUp();
    }
}

void ConsoleManager::render()
{
    d_console_window.clear();
    d_console_window.pushGLStates();
    gui.draw();
    d_console_window.display();
    d_console_window.popGLStates();
}

void ConsoleManager::translateCommandsUp()
{
    d_console_command_list->
        addLine(d_console_edit_box->getText().getData());
    d_console_edit_box->setText("> ");
}

void ConsoleManager::printToConsole(const std::string& text) 
{
    //a very weird way parsing the input and adding new lines to the console when a '%' is found
    auto Iter = std::find(text.begin(), text.end(), '%');
    std::string line = text.substr(0,std::distance(text.begin(), Iter));
    if(Iter != text.end())
        Iter++;
    d_console_command_list->addLine(sf::String(line));
    while(1) {
        std::size_t start = std::distance(text.begin(),Iter);
        auto IterEnd = std::find(text.begin()+start, text.end(), '%');

        if(IterEnd == text.end()) {
            std::string sline = text.substr(std::distance(text.begin(), text.begin()+start), std::distance(text.begin()+start, IterEnd));
            d_console_command_list->addLine(sf::String(sline));
            return;
        }

        std::string sline = text.substr(std::distance(text.begin(),text.begin()+start), std::distance(text.begin()+start,IterEnd));

        Iter = IterEnd;
        ++Iter;

        d_console_command_list->addLine(sf::String(sline));

    }
}

void ConsoleManager::handleSetCommand(const std::string& str)
{
    consolecommands::VarKey key;
    if(!consolecommands::isValidCommandVariable(str, key)) {
        std::stringstream ss;
        ss << "Variable " << str << " not found";
        printToConsole(ss.str());
    }
}

void ConsoleManager::handleGetCommand(const std::string& str)
{

}