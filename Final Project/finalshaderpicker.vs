#version 410 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

in vec3 in_Position;
in vec3 in_Normal;

main(){
	gl_position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);
}