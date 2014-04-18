// Include SFML
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"
#include "math.h"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix = glm::perspective(45.0f, 4.0f/3.0f, 0.1f, 100.0f);
glm::mat4 ModelMatrix = glm::mat4(1.0f);

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}

glm::mat4 getModelMatrix(){
	return ModelMatrix;
}

glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}


// Initial position : on +Z
glm::vec3 position = glm::vec3( 0, 0, 5 ); 
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
//direction variable
glm::vec3 direction(0.0);
//how right vector
glm::vec3 right(0.0);
//speed of player moving around
float speed = 7.0f;
//mouse speed
float mouseSpeed = 0.005f;

void computeMatricesFromInputs(sf::Window& window, float time){
	//get mouse position
	sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		//set position of mouse so it does not move at all
	sf::Mouse::setPosition(sf::Vector2i(400, 300), window);

	//since the mouse position is always set to the center of the screen, any slight movements
	//	are added to the horizontal and vertical angle. (400,300) must also equal zero for the 
	//	cube to remain still at origin, so that's why the mouse position is the subtractor to
	//	the center of the screen
	horizontalAngle += mouseSpeed * (800/2 - mousePos.x);
	verticalAngle   += mouseSpeed * (600/2 - mousePos.y);

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	direction = glm::vec3(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	
	// Right vector
	right = glm::vec3(
		sin(horizontalAngle - 3.14f/2.0f), 
		0,
		cos(horizontalAngle - 3.14f/2.0f)
	);
	// Up vector
	glm::vec3 up = glm::cross( right, direction );
	
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,	//camera position
		position+direction,	//look at origin
		up	//head up
	);

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
		position += direction * time * speed;
	}else if(sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
		position -= direction * time * speed;
	}else if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
		position += right * time * speed;
	}else if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
		position -= right * time * speed;
	}

	//beta implementation of moving model in object space
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-0.1f,0.0f,0.0f));
	}else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.1f,0.0f,0.0f));
	}else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f,0.1f,0.0f));
	}else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f,-0.1f,0.0f));
	}
}