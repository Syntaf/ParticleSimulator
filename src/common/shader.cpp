#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <GL/glew.h>
#include "shader.hpp"
#include "../guiconsole/console.hpp"

using namespace std;

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path, 
                   ConsoleManager *out){

    bool error_happened = false;

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    const unsigned char* version = (const unsigned char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    std::stringstream ss;
    ss << "running GLSL Version " << version;
    out->printToConsole(ss.str());
    ss.clear();
    ss.str(std::string());

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open()){
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }else{
        ss << "Impossible to open " << vertex_file_path << ". Are you in the right directory?";
        out->printToConsole(ss.str());
        ss.clear();
        ss.str(std::string());
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    ss << "Compiling shader : " << vertex_file_path;
    out->printToConsole(ss.str());
    ss.clear();
    ss.str(std::string());
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        ss << &VertexShaderErrorMessage[0];
        if(ss.rdbuf()->in_avail() > 0) {
            error_happened = true;
            out->printToConsole(ss.str());
            ss.clear();
            ss.str(std::string());
    
        }
    }



    // Compile Fragment Shader
    ss << "Compiling shader : " << fragment_file_path;
    out->printToConsole(ss.str());
    ss.clear();
    ss.str(std::string());
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        ss << &FragmentShaderErrorMessage[0];
        if(ss.rdbuf()->in_avail() > 0) {
            error_happened = true;
            out->printToConsole(ss.str());
            ss.clear();
            ss.str(std::string());
        }
    }



    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        ss << &ProgramErrorMessage[0];
        if(ss.rdbuf()->in_avail() > 0) {
            out->printToConsole(ss.str());
            ss.clear();
            ss.str(std::string());
        }
    }
    //move text to top of console
    if(error_happened)
        out->printToConsole("%%%%");
    else
        out->printToConsole("%%%%%%%%");
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}
