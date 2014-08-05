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
        "help"
    };

    enum Key {
        EXIT=0,
        SET,
        GET,
        HELP
    };

    bool isValidCommandKey(std::string key, Key& k) 
    {
        for(std::size_t i=0; i<sizeof(valid_command)/sizeof(std::string); i++) {
            if (key == valid_command[i]) {
                k = Key(i);
                return true;
            }
        }
        return false;
    }
}

#endif