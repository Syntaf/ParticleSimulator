#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <GL/glew.h>

// Load a .BMP file using our custom loader
GLuint loadBMP_custom(const char * imagepath);

// Load a .TGA file using sfml's own loader
GLuint loadTGA_sfml(const char * imagepath);

// Load a .DDS file using GLFW's own loader
GLuint loadDDS(const char * imagepath);


#endif