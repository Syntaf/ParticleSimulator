#ifndef CONSOLECOMMANDS_H_
#define CONSOLECOMMANDS_H_

#include <string>
#include <algorithm>

namespace consolecommands {
    //valid console commands
    std::string valid_command[] = {
        "exit",
        "set",
        "get"
    };

    bool isValidCommandKey(std::string key) 
    {
        for(std::size_t i=0; i<sizeof(valid_command)/sizeof(std::string); i++) {
            if (key == valid_command[i])
                return true;
        }
        return false;
    }
}

#endif