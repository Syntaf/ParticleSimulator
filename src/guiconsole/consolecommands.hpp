#ifndef CONSOLECOMMANDS_H_
#define CONSOLECOMMANDS_H_

#include <string>
#include <algorithm>

namespace consolecommands {
    //valid console commands
    std::string valid_command[] = {
        "exit",
        "set",
        "get",
        "help",
        "reset"
    };

    std::string valid_variables[] = {
        "drag",
        "mass",
        "mouseforce"
    };

    enum Key {
        EXIT=0,
        SET,
        GET,
        HELP,
        RESET
    };

    enum VarKey {
        DRAG=0,
        MASS,
        MOUSEFORCE
    };

    bool isValidCommandKey(const std::string& key, Key& k) 
    {
        for(std::size_t i=0; i<sizeof(valid_command)/sizeof(std::string); i++) {
            if (key == valid_command[i]) {
                k = Key(i);
                return true;
            }
        }
        return false;
    }

    bool isValidCommandVariable(const std::string& key, VarKey& k)
    {
        for(std::size_t i=0; i<sizeof(valid_variables)/sizeof(std::string); i++) {
            if (key == valid_variables[i]) {
                k = VarKey(i);
                return true;
            }
        }
        return false;
    }
}

#endif