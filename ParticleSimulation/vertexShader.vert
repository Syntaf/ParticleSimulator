#version 330 core

//Input vertex data
layout(location =0) in vec3 squareVertices;
layout(location =1) in vec4 xyzs;			//Position of center of particle and size of square

uniform vec3 cameraRight_worldspace;
uniform vec3 cameraUp_worldspace;
uniform mat4 VP;

void main() {
	float particleSize = xyzs.w; //because I said so
	vec3 particleCenter_worldspace = xyzs.xyz;

	vec3 vertexPosition_worldspace =
		particleCenter_worldspace
		+ cameraRight_worldspace * squareVertices.x * particleSize
		+ cameraUp_worldspace * squareVertices.y * particleSize;

	//Output position of vertex
	gl_Position = VP * vec4(vertexPosition_worldspace, 1.0f);
}