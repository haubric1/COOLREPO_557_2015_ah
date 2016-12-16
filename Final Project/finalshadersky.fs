#version 410 core

in vec3 texcoords;
uniform samplerCube cube_texture;
out vec4 frag_colour;

void main() {
	frag_colour = texture(cube_texture, texcoords);
}