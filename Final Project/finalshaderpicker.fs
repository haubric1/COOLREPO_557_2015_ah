#version 410 core

uniform int code;

out vec4 outputF;

main{
	outputF = vec4((float)code/255.0, 1, 1, 1);
}