#ifndef SHADER_HPP
#define SHADER_HPP

#include "../guiconsole/console.hpp"

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path,
                   ConsoleManager *out);

#endif
