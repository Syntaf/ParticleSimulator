#ifndef CONSOLECOMMANDS_H_
#define CONSOLECOMMANDS_H_

#include <string>
#include <algorithm>

//valid console commands
std::string valid_command[] = {
    "exit",
    "set",
    "get"
};

bool isValidCommandKey(std::string key) 
{
    int comm = std::distance(key.begin(), std::find(key.begin(), key.end(), ' '));
    std::cout << comm << std::endl;
    return false;
}

#endif