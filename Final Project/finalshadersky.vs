#version 410 core

in vec3 vertices;

uniform mat4 projectionMatrix, viewMatrix;

out vec3 texcoords;

void main(){
	texcoords = vertices;
	gl_Position = projectionMatrix * viewMatrix * vec4(vertices, 1.0);
}