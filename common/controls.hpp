#ifndef CONTROLS_HPP
#define CONTROLS_HPP

#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>

void computeMatricesFromInputs(sf::Window& window, float time);
void handleKeyboard(float time);
glm::mat4 getViewMatrix();
glm::mat4 getModelMatrix();
glm::mat4 getProjectionMatrix();

#endif