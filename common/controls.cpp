// Include SFML
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "controls.hpp"
#include "math.h"

using namespace glm;

const float speed = 7.0f;					// speed of camera moving around 
const float lookSpeed = 0.005f;				// mouse speed

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix = glm::perspective(45.0f, 4.0f/3.0f, 0.1f, 100.0f);
glm::mat4 ModelMatrix = glm::mat4(1.0f);
glm::vec3 position = glm::vec3( 0, 0, 5 );  // Initial position : on +Z
float horizontalAngle = 3.14f;				// Initial horizontal angle : toward -Z
float verticalAngle = 0.0f;					// Initial vertical angle : none
glm::vec3 direction(0.0);					// camera direction vector
glm::vec3 right(0.0);						// right vector used for lookat()


//get matricies
glm::mat4 getViewMatrix(){return ViewMatrix;}
glm::mat4 getModelMatrix(){return ModelMatrix;}
glm::mat4 getProjectionMatrix(){return ProjectionMatrix;}

void computeMatricesFromInputs(sf::Window& window, float time){

	// Looking around, think of just a head pivoting
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){
		horizontalAngle += lookSpeed * speed;
	}else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
		horizontalAngle += lookSpeed * speed * -1;
	}else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){
		verticalAngle += lookSpeed * speed;
	}else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){
		verticalAngle += lookSpeed * speed * -1;
	}

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

	// moving around, think of a person walking left/right/forward/backward
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
		position += direction * time * speed;
	}else if(sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
		position -= direction * time * speed;
	}else if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
		position += right * time * speed;
	}else if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
		position -= right * time * speed;
	}
}