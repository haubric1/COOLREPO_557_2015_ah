//
//  main.cpp
//  OpenGL4Test
//
//  Created by Rafael Radkowski on 5/28/15.
//  Copyright (c) 2015 -. All rights reserved.
//

// stl include
#include <iostream>
#include <string>
#include <algorithm>
#include <map>

// GLEW include
#include <GL/glew.h>

// GLM include files
#define GLM_FORCE_INLINE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


// glfw includes
#include <GLFW/glfw3.h>


// include local files
#include "controls.h"
#include "HCI557Common.h"
#include "CoordSystem.h"
#include "make_sphere.h"
#include "ShaderFileUtils.h"
#include "ImgLoader.h"

#define PI 3.14159265

// this line tells the compiler to use the namespace std.
// Each object, command without a namespace is assumed to be part of std. 
using namespace std;

/// Camera control matrices
glm::mat4 projectionMatrix; // Store the projection matrix
glm::mat4 viewMatrix; // Store the view matrix
glm::mat4 modelMatrix; // Store the model matrix
glm::mat4 inverseViewMatrix;

// The handle to the window object
GLFWwindow*         window;

// Define some of the global variables we're using for this sample
static const string program_vs_loc = "C:\\Users\\Arlo\\Desktop\\Stuff\\IAState Class Materials\\557\\Projects\\Homework_2\\02_3D_Modeling\\02_3D_Modeling\\finalshader.vs";
static const string program_fs_loc = "C:\\Users\\Arlo\\Desktop\\Stuff\\IAState Class Materials\\557\\Projects\\Homework_2\\02_3D_Modeling\\02_3D_Modeling\\finalshader.fs";
static const string program_vs_picker = "C:\\Users\\Arlo\\Desktop\\Stuff\\IAState Class Materials\\557\\Projects\\Homework_2\\02_3D_Modeling\\02_3D_Modeling\\finalshaderpicker.vs";
static const string program_fs_picker = "C:\\Users\\Arlo\\Desktop\\Stuff\\IAState Class Materials\\557\\Projects\\Homework_2\\02_3D_Modeling\\02_3D_Modeling\\finalshaderpicker.fs";
static const string program_vs_sky = "C:\\Users\\Arlo\\Desktop\\Stuff\\IAState Class Materials\\557\\Projects\\Homework_2\\02_3D_Modeling\\02_3D_Modeling\\finalshadersky.vs";
static const string program_fs_sky = "C:\\Users\\Arlo\\Desktop\\Stuff\\IAState Class Materials\\557\\Projects\\Homework_2\\02_3D_Modeling\\02_3D_Modeling\\finalshadersky.fs";

// Define the locations of our skybox textures
static const char* skybox_front = "C:\\Users\\Arlo\\Desktop\\Stuff\\IAState Class Materials\\557\\Projects\\Homework_2\\02_3D_Modeling\\02_3D_Modeling\\stars1.bmp";
static const char* skybox_right = "C:\\Users\\Arlo\\Desktop\\Stuff\\IAState Class Materials\\557\\Projects\\Homework_2\\02_3D_Modeling\\02_3D_Modeling\\stars2.bmp";
static const char* skybox_left = "C:\\Users\\Arlo\\Desktop\\Stuff\\IAState Class Materials\\557\\Projects\\Homework_2\\02_3D_Modeling\\02_3D_Modeling\\stars3.bmp";
static const char* skybox_top = "C:\\Users\\Arlo\\Desktop\\Stuff\\IAState Class Materials\\557\\Projects\\Homework_2\\02_3D_Modeling\\02_3D_Modeling\\stars4.bmp";
static const char* skybox_bottom = "C:\\Users\\Arlo\\Desktop\\Stuff\\IAState Class Materials\\557\\Projects\\Homework_2\\02_3D_Modeling\\02_3D_Modeling\\stars5.bmp";
static const char* skybox_back = "C:\\Users\\Arlo\\Desktop\\Stuff\\IAState Class Materials\\557\\Projects\\Homework_2\\02_3D_Modeling\\02_3D_Modeling\\stars6.bmp";

static const string environment_map = "C:\\Users\\Arlo\\Desktop\\Stuff\\IAState Class Materials\\557\\Projects\\Homework_2\\02_3D_Modeling\\02_3D_Modeling\\tex_1.bmp";

// Define the locations we'll store references to our shaders in.
GLuint program, skybox;

// Number of vertices for our sphere
int number_vertices_PC = -1;
int number_vertices_b = -1;

// Chooses which, if any, planet to highlight.
bool highlight_pc = false;
bool highlight_b = false;
bool lock_camera = true;

// Chooses whether we look at origin or b.
bool follow_b = false;

// Holds mouse location.
double xx = 0;
double yy = 0;
float select_index = 0;
bool mouse_release = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Fill this functions with your model code.

// USE THESE vertex array objects to define your objects
unsigned int vaoID[3];

unsigned int vboID[4];

//Struct definitions
typedef struct _keyframe {
	float _t;
	glm::vec3 _p;

	_keyframe(float t, glm::vec3 p) {
		_t = t;
		_p = p;
	}

	_keyframe() {
		_t = -1.0;
		_p = glm::vec3(0.0,0.0,0.0);
	}

}Keyframe;

typedef map<double, Keyframe> keyframeAnimation;

bool load_cube_map_side(
	GLuint texture, GLenum side_target, const char* file_name) {
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	int x, y, n;
	x = 256;
	y = 256;
	int force_channels = 4;
	unsigned char*  image_data = ImgLoader::Load(file_name);
	if (!image_data) {
		fprintf(stderr, "ERROR: could not load %s\n", file_name);
		return false;
	}
	// non-power-of-2 dimensions check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf(stderr,
			"WARNING: image %s is not power-of-2 dimensions\n",
			file_name);
	}

	// copy image data into 'target' side of cube map
	glTexImage2D(
		side_target,
		0,
		GL_RGB,
		x,
		y,
		0,
		GL_BGR,
		GL_UNSIGNED_BYTE,
		image_data);
	//free(image_data);
	return true;
}

//Model initialization functions.

/*!
This function creates a sphere model.
*/
int createBody(float r, int index)
{
	
	//set the center and radius of the sphere
	float center[3] = { 0.0, 0.0, 0.0 };
	float radius = r;

	//set the number of rows, and the number of segments per row
	int rows = 40;
	int segments = 40;

	//compute the number of segments needed for storage ((segments * 2) + 1) * 6 * rows
	int N = NumVec3ArrayElements(rows, segments);

	float* points = new float[N];
	float* normals = new float[N];

	int body_vertices = Make_Sphere(rows, segments, center, radius, points, normals);

	glGenVertexArrays(1, &vaoID[index]); //creating vertex array objects for the triangle strips model
	glBindVertexArray(vaoID[index]); //binding it!

	glGenBuffers(2, vboID); //generate our vertex buffer object

	//vertices!
	glBindBuffer(GL_ARRAY_BUFFER, vboID[0]); //bind our VBO
	glBufferData(GL_ARRAY_BUFFER, N * sizeof(float), points, GL_STATIC_DRAW);

	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);//disable the vertex array

	//Colors!
	glBindBuffer(GL_ARRAY_BUFFER, vboID[1]); //bind our VBO
	glBufferData(GL_ARRAY_BUFFER, N * sizeof(float), normals, GL_STATIC_DRAW);

	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);//disable the vertex array

	glBindVertexArray(0);

	return body_vertices;
}
/*!
Creates a skybox model and binds it to index 0.
*/
void createSkybox(void) {
	float points[] = {
		-10.0f,  10.0f, -10.0f,
		-10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,

		-10.0f, -10.0f,  10.0f,
		-10.0f, -10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f,  10.0f,
		-10.0f, -10.0f,  10.0f,

		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,

		-10.0f, -10.0f,  10.0f,
		-10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f, -10.0f,  10.0f,
		-10.0f, -10.0f,  10.0f,

		-10.0f,  10.0f, -10.0f,
		10.0f,  10.0f, -10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		-10.0f,  10.0f,  10.0f,
		-10.0f,  10.0f, -10.0f,

		-10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f,  10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f,  10.0f,
		10.0f, -10.0f,  10.0f
	};
	glGenBuffers(1, vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
	glBufferData(GL_ARRAY_BUFFER, 3 * 36 * sizeof(float), &points, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vaoID[0]);
	glBindVertexArray(vaoID[0]);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}
/*!
This function makes a sphere model for b and loads it into vaoID[2]
*/
void createB(void) {
	number_vertices_b = createBody(0.5, 2);
	return;
}
/*!
This function makes a sphere model for Proxima Centauri and loads in into vaoID[1]
*/
void createPC(void) {
	number_vertices_PC = createBody(1.0, 1);

	unsigned char* data1 = ImgLoader::Load(environment_map);

	int texLoc = glGetUniformLocation(program, "tex");

	glActiveTexture(GL_TEXTURE1);

	GLuint _texture1;
	glGenTextures(0, &_texture1);

	//setting it as the active texture
	glBindTexture(GL_TEXTURE_2D, _texture1);
	glUniform1i(texLoc, 1);

	//Setting our texture units up
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//setting our texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenerateMipmap(GL_TEXTURE_2D);

	//Load the texture to the graphics hardware
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_BGR, GL_UNSIGNED_BYTE, data1);

	return;
}

/*!
This function creates a cube map from six textures.
*/
void create_cube_map(
	const char* front,
	const char* back,
	const char* top,
	const char* bottom,
	const char* left,
	const char* right,
	GLuint* tex_cube) {
	// generate a cube-map texture to hold all the sides
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, tex_cube);
	int texloc = glGetUniformLocation(skybox, "cube_texture");
	glUniform1i(texloc, 0);
	// load each image and copy into a side of the cube-map texture
	load_cube_map_side(*tex_cube, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, front);
	load_cube_map_side(*tex_cube, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, back);
	load_cube_map_side(*tex_cube, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, top);
	load_cube_map_side(*tex_cube, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, bottom);
	load_cube_map_side(*tex_cube, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, left);
	load_cube_map_side(*tex_cube, GL_TEXTURE_CUBE_MAP_POSITIVE_X, right);
	// format cube map texture
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

/*!
This function links the shader made from the compiled visual and fragment shaders at vs_loc and fs_loc and returns the linked GLuint
*/
GLuint compileShader(string vs_loc, string fs_loc) {
	// Vertex shader source code. This draws the vertices in our window. We have 3 vertices since we're drawing an triangle.
	// Each vertex is represented by a vector of size 4 (x, y, z, w) coordinates.
	static const string vertex_code = ShaderFileUtils::LoadFromFile(vs_loc);
	static const char * vs_source = vertex_code.c_str();

	// Fragment shader source code. This determines the colors in the fragment generated in the shader pipeline. In this case, it colors the inside of our triangle specified by our vertex shader.
	static const string fragment_code = ShaderFileUtils::LoadFromFile(fs_loc);
	static const char * fs_source = fragment_code.c_str();

	// This next section we'll generate the OpenGL program and attach the shaders to it so that we can render our triangle.
	GLuint program = glCreateProgram();

	// We create a shader with our fragment shader source code and compile it.
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_source, NULL);
	glCompileShader(fs);

	// We create a shader with our vertex shader source code and compile it.
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_source, NULL);
	glCompileShader(vs);

	// We'll attach our two compiled shaders to the OpenGL program.
	glAttachShader(program, vs);
	glAttachShader(program, fs);

	glLinkProgram(program);

	return program;
}

/*!
This function renders a sphere.
*/
void renderTriangleStripModel(int number_vertices, int index)
{

	// Bind the buffer and switch it to an active buffer
	glBindVertexArray(vaoID[index]);

	// Draw the triangles
	glDrawArrays(GL_TRIANGLE_STRIP, 0, number_vertices);

	// Unbind our Vertex Array Object
	glBindVertexArray(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*!
This function creates the models.
*/
void setupScene(void) {
	
	createPC();
	createB();
	//createTriangleStripModel();

}

/*void renderSelection(void) {
	
	glClearColor(0.0, 0.0, 0.0, 0.0);

	int codeLocation = glGetUniformLocation(program, "code");

	glUniform1i(codeLocation, 1);
	
	renderTriangleStripModel(number_vertices_PC, 1);

	glUniform1i(codeLocation, 2);

	renderTriangleStripModel(number_vertices_b, 2);

}*/

/*void processSelection(double xx, double yy) {
	
	unsigned char res[4];
	GLint viewport[4];
	glUseProgram(picker);
	glEnable(GL_SCISSOR_TEST);
	glScissor(xx, viewport[3] - yy, 1, 1);
	renderSelection();
	glGetIntegerv(GL_VIEWPORT, viewport);
	glReadPixels(xx, viewport[3] - yy, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &res);
	switch (res[0]) {
		case 0: printf("Cursor is at %f, %f. Nothing picked.\n", xx, yy); break;
		case 1: printf("Cursor is at %f, %f. Picked Proxima Centauri.\n", xx, yy); break;
		case 2: printf("Cursor is at %f, %f. Picked b.\n", xx, yy); break;
		default: printf("Cursor is at %f, %f. Picked %d\n", xx, yy, res[0]);
	}
	glDisable(GL_SCISSOR_TEST);
	glUseProgram(0);
}*/

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	xx = xpos;
	yy = ypos;
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		mouse_release = true;
	}
}

keyframeAnimation make_b_keyframes(void) {
	keyframeAnimation bKeyframes;
	int i;
	for (i = 0; i < 1001; i++) {
		float time = (float)i / 1000.0;
		double xpos = cos(time * 2 * PI) * 5;
		double zpos = sin(time * 2 * PI) * 5;
		bKeyframes[time] = Keyframe(time, glm::vec3(xpos, 0.0, zpos));
	}
	return bKeyframes;
};

float getTimeFraction(float time, float duration) {
	float interval = floor(time / duration);
	float curr_interval = time - interval*duration;
	return curr_interval / duration;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		follow_b = !follow_b;
	}
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
		if (highlight_pc) {
			highlight_pc = false;
			highlight_b = true;
		}
		else if(highlight_b) {
			highlight_b = false;
		}
		else {
			highlight_pc = true;
		}
	}
	if (key = GLFW_KEY_RIGHT_SHIFT && action == GLFW_PRESS) {
		lock_camera = !lock_camera;
	}
}

int getKeyframes(keyframeAnimation& keyframes, const double time, Keyframe& k0, Keyframe& k1) {
	int num_keyframes = 0;

	keyframeAnimation::iterator k_itr = keyframes.lower_bound(time);

	k1 = (*k_itr).second; num_keyframes++;

	if (k_itr != keyframes.begin()) {
		k_itr--;
		k0 = (*k_itr).second; num_keyframes++;
	}
	else {
		k0 = k1;
	}

	return num_keyframes;
}

bool interpolateKeyframe(const float fraction, const Keyframe &k0, const Keyframe &k1, Keyframe &res) {
	//check if these are the same frame, by checking if they got the same time.
	float delta_t = k1._t - k0._t;
	if (delta_t == 0.0f) {
		res = k0;
		return true;
	}

	//Interpolate the position
	glm::vec3 delta_p = k1._p - k0._p;
	glm::vec3 p_int = k0._p + delta_p * (fraction - k0._t)/(delta_t);

	res = Keyframe(fraction, p_int);
	return true;
}

double get_b_pos(int index, double time) {
	switch (index) {
		case 0: return cos(time / 100 * 360) * 5; break;
		case 1: return sin(time / 100 * 360) * 5; break;
		default: return 0; break;
	}
};

/*!
This renders my expertly-crafted skybox.
*/
void renderSkybox(GLuint tex_cube, glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
	glDepthMask(GL_FALSE);
	glUseProgram(skybox);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_cube);
	glBindVertexArray(vaoID[0]);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);
	glUseProgram(0);
}

/*!
This renders Proxima Centauri with the selected shader.
*/
void renderPC(GLuint program) {

	int modelMatrixLocation = glGetUniformLocation(program, "modelMatrix"); // Get the location of our model matrix in the shader
	int lightPositionLocation = glGetUniformLocation(program, "light_position");
	int ambientColorLocation = glGetUniformLocation(program, "ambient_color");
	int diffuseColorLocation = glGetUniformLocation(program, "diffuse_color");
	int specularColorLocation = glGetUniformLocation(program, "specular_color");
	int ambientIntensityLocation = glGetUniformLocation(program, "ambient_intensity");
	int diffuseIntensityLocation = glGetUniformLocation(program, "diffuse_intensity");
	int specularIntensityLocation = glGetUniformLocation(program, "specular_intensity");
	int shininessLocation = glGetUniformLocation(program, "shininess");
	int attenuationCoefficientLocation = glGetUniformLocation(program, "attenuation_coefficient");
	int coneAngleLocation = glGetUniformLocation(program, "cone_angle");
	int coneDirectionLocation = glGetUniformLocation(program, "cone_direction");
	int envMapLocation = glGetUniformLocation(program, "env_map");
	int selectModeLocation = glGetUniformLocation(program, "select_mode");

	//This puts Proxima Centauri at the center.
	modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);

	//Turning on environment mapping.
	glUniform1i(envMapLocation, 1);

	//Set shader settings for the first sphere
	//Setting the light position, cone angle, cone direction, and attenuation
	glm::vec3 lightPos = glm::vec3(20.0, 0.0, 0.0);
	glm::vec3 coneDirection = glm::vec3(1.0, 0.0, 0.0);
	float coneAngle = 360.0;
	float attenuationCoefficient = 0.00;
	glUniform3fv(lightPositionLocation, 1, &lightPos[0]);
	glUniform3fv(coneDirectionLocation, 1, &coneDirection[0]);
	glUniform1f(coneAngleLocation, coneAngle);
	glUniform1f(attenuationCoefficientLocation, attenuationCoefficient);

	//Setting the ambient color and intensity
	float ambientIntensity = 0.2f;
	if (highlight_pc) {
		ambientIntensity = 0.8f;
	}
	glm::vec3 ambientColor = glm::vec3(1.0, 0.05, 0.0);
	glUniform1f(ambientIntensityLocation, ambientIntensity);
	glUniform3fv(ambientColorLocation, 1, &ambientColor[0]);

	//setting the diffuse color and intensity
	float diffuseIntensity = 0.3f;
	glm::vec3 diffuseColor = glm::vec3(1.0, 0.0, 0.0);
	glUniform1f(diffuseIntensityLocation, diffuseIntensity);
	glUniform3fv(diffuseColorLocation, 1, &diffuseColor[0]);

	//setting the specular color, intensity, and shininess
	float specularIntensity = 0.0f;
	if (highlight_pc) specularIntensity = 0.5f;
	glm::vec3 specularColor = glm::vec3(1.0, 1.0, 1.0);
	float shininess = 0.5f;
	glUniform1f(specularIntensityLocation, specularIntensity);
	glUniform1f(shininessLocation, shininess);
	glUniform3fv(specularColorLocation, 1, &specularColor[0]);

	glUniform1i(selectModeLocation, 0);

	//render PC.
	renderTriangleStripModel(number_vertices_PC, 1);

}

void renderPCSelect(GLuint program) {

	int modelMatrixLocation = glGetUniformLocation(program, "modelMatrix"); // Get the location of our model matrix in the shader
	int envMapLocation = glGetUniformLocation(program, "env_map");
	int selectModeLocation = glGetUniformLocation(program, "select_mode");
	int selectColorLocation = glGetUniformLocation(program, "select_color");

	//This puts Proxima Centauri at the center.
	modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);

	//Turning off environment mapping.
	glUniform1i(envMapLocation, 0);

	glUniform1i(selectModeLocation, 1);
	glm::vec3 selectColor = glm::vec3(1.0, 0.0, 0.0);
	glUniform3fv(selectColorLocation, 1, &selectColor[0]);

	//render PC.
	renderTriangleStripModel(number_vertices_PC, 1);

}

/*!
This renders b, the planet in Proxima Centauri's system.
*/
void renderB(GLuint program, Keyframe &frame) {

	int modelMatrixLocation = glGetUniformLocation(program, "modelMatrix"); // Get the location of our model matrix in the shader
	int lightPositionLocation = glGetUniformLocation(program, "light_position");
	int ambientColorLocation = glGetUniformLocation(program, "ambient_color");
	int diffuseColorLocation = glGetUniformLocation(program, "diffuse_color");
	int specularColorLocation = glGetUniformLocation(program, "specular_color");
	int ambientIntensityLocation = glGetUniformLocation(program, "ambient_intensity");
	int diffuseIntensityLocation = glGetUniformLocation(program, "diffuse_intensity");
	int specularIntensityLocation = glGetUniformLocation(program, "specular_intensity");
	int shininessLocation = glGetUniformLocation(program, "shininess");
	int attenuationCoefficientLocation = glGetUniformLocation(program, "attenuation_coefficient");
	int coneAngleLocation = glGetUniformLocation(program, "cone_angle");
	int coneDirectionLocation = glGetUniformLocation(program, "cone_direction");
	int envMapLocation = glGetUniformLocation(program, "env_map");
	int selectModeLocation = glGetUniformLocation(program, "select_mode");

	//Turning off environment mapping.
	glUniform1i(envMapLocation, 0);

	// This sets b far from the sun.
	modelMatrix = glm::translate(glm::mat4(1.0f), frame._p);
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]); // Send our model matrix to the shader

	//Set shader settings for the second sphere
	//Setting the light position, cone direction, cone angle, and attenuation
	glm::vec3 lightPos = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 coneDirection = glm::vec3(0.0, 0.0, 1.0);
	float coneAngle = 360.0f;
	float attenuationCoefficient = 0.0f;
	glUniform3fv(lightPositionLocation, 1, &lightPos[0]);
	glUniform3fv(coneDirectionLocation, 1, &coneDirection[0]);
	glUniform1f(coneAngleLocation, coneAngle);
	glUniform1f(attenuationCoefficientLocation, attenuationCoefficient);

	//Setting the ambient color and intensity
	float ambientIntensity = 0.2f;
	if (highlight_b) {
		ambientIntensity = 1.2f;
	}
	glm::vec3 ambientColor = glm::vec3(0.0, 0.0, 1.0);
	glUniform1f(ambientIntensityLocation, ambientIntensity);
	glUniform3fv(ambientColorLocation, 1, &ambientColor[0]);

	//setting the diffuse color and intensity
	float diffuseIntensity = 0.5f;
	glm::vec3 diffuseColor = glm::vec3(1.0, 0.0, 0.0);
	glUniform1f(diffuseIntensityLocation, diffuseIntensity);
	glUniform3fv(diffuseColorLocation, 1, &diffuseColor[0]);

	//setting the specular color, intensity, and shininess
	float specularIntensity = 0.5f;
	if (highlight_b) specularIntensity = 3.5f;
	glm::vec3 specularColor = glm::vec3(1.0, 0.0, 0.0);
	float shininess = 0.25f;
	glUniform1f(specularIntensityLocation, specularIntensity);
	glUniform1f(shininessLocation, shininess);
	glUniform3fv(specularColorLocation, 1, &specularColor[0]);

	glUniform1i(selectModeLocation, 0);

	// This line renders b.
	renderTriangleStripModel(number_vertices_b, 2);
}

void renderBSelect(GLuint program, Keyframe &frame) {

	int modelMatrixLocation = glGetUniformLocation(program, "modelMatrix"); // Get the location of our model matrix in the shader
	int envMapLocation = glGetUniformLocation(program, "env_map");
	int selectModeLocation = glGetUniformLocation(program, "select_mode");
	int selectColorLocation = glGetUniformLocation(program, "select_color");

	//This puts Proxima Centauri at the center.
	modelMatrix = glm::translate(glm::mat4(1.0f), frame._p);
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);

	//Turning off environment mapping.
	glUniform1i(envMapLocation, 0);

	glUniform1i(selectModeLocation, 1);
	glm::vec3 selectColor = glm::vec3(1.0, 1.0, 0.0);
	glUniform3fv(selectColorLocation, 1, &selectColor[0]);

	//render PC.
	renderTriangleStripModel(number_vertices_b, 2);

}

/*!
Renders the objects in select mode and returns the index of the object.
0.1 - Proxima Centauri
0.2 - it's b!
*/
float getSelection(Keyframe &frame) {
	//array to hold our color data.
	float col[3];
	GLint xpos = (GLint)xx;
	GLint ypos = (GLint)600 - (GLint)yy;
	//rendering our objects in select mode using a scissor test.
	glUseProgram(program);
	glEnable(GL_SCISSOR_TEST);
	glScissor(xpos, ypos, 1, 1);
	renderPCSelect(program);
	renderBSelect(program, frame);
	glUseProgram(0);
	glDisable(GL_SCISSOR_TEST);

	//Getting the index from our pixel.
	glReadPixels(xx, 600 - yy, 1, 1, GL_RGB, GL_FLOAT, &col);
	return col[0] + col[1];
}

/*!
The main function.
*/
int main(int argc, const char * argv[])
{

	// Init the GLFW Window
	window = initWindow();

	// Init the glew api
	initGlew();

	// Prepares some defaults
	// Prepare the shaders we're going to be using for picking, rendering the skybox, and rendering the bodies.
	program = compileShader(program_vs_loc, program_fs_loc);
	//picker = compileShader(program_vs_picker, program_fs_picker);
	//skybox = compileShader(program_vs_sky, program_fs_sky);
	//GLuint skytex;

	//Setting up our skybox texture.
	/*createSkybox();
	create_cube_map(skybox_front, skybox_back, skybox_top, skybox_bottom, skybox_left, skybox_right, &skytex);*/

	// We'll specify that we want to use this program that we've attached the shaders to.


	// Set up our green background color
	static const GLfloat clear_color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	static const GLfloat clear_depth[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	//setting up our projection, model, and view matrices
	projectionMatrix = glm::perspective(1.1f, (float)800 / (float)600, 0.1f, 100.f);
	modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)); // Create our model matrix which will halve the size of our model
	viewMatrix = glm::lookAt(glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	inverseViewMatrix = glm::inverse(viewMatrix);

	//Getting locations in our shader program to send values to
	glUseProgram(program);
	int projectionMatrixLocation = glGetUniformLocation(program, "projectionMatrix"); // Get the location of our projection matrix in the shader
	int viewMatrixLocation = glGetUniformLocation(program, "viewMatrix"); // Get the location of our view matrix in the shader
	int modelMatrixLocation = glGetUniformLocation(program, "modelMatrix"); // Get the location of our model matrix in the shader
	int inverseViewLocation = glGetUniformLocation(program, "inverseViewMatrix");

	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]); // Send our projection matrix to the shader
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]); // Send our view matrix to the shader
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]); // Send our model matrix to the shader
	glUniformMatrix4fv(inverseViewLocation, 1, GL_FALSE, &inverseViewMatrix[0][0]);

	/*glUseProgram(skybox);
	int skyboxViewMatrixLocation = glGetUniformLocation(skybox, "viewMatrix");
	int skyboxProjectionMatrixLocation = glGetUniformLocation(skybox, "projectionMatrix");
	int skyboxCubeLocation = glGetUniformLocation(skybox, "cube_texture");

	glUniformMatrix4fv(skyboxProjectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]); // Send our projection matrix to the shader
	glUniformMatrix4fv(skyboxViewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]); // Send our view matrix to the shader*/
	/*
	glUseProgram(picker);
	int pickerProjectionMatrixLocation = glGetUniformLocation(picker, "projectionMatrix");
	int pickerViewMatrixLocation = glGetUniformLocation(picker, "viewMatrix");
	int pickerModelMatrixLocation = glGetUniformLocation(picker, "modelMatrix");
	int pickerCodeLocation = glGetUniformLocation(picker, "code");
	
	glUniformMatrix4fv(pickerProjectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]); // Send our projection matrix to the shader
	glUniformMatrix4fv(pickerViewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]); // Send our view matrix to the shader
	glUniformMatrix4fv(pickerModelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]); // Send our model matrix to the shader
	*/
	// this creates the scene
	setupScene(); 
	keyframeAnimation bKeyframes = make_b_keyframes();
	//glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glm::mat4 rotated_view = viewMatrix;
	//glm::mat4 skybox_view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec4 view_mover = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	// Enable depth test
	// ignore this line, it allows us to keep the distance value after we proejct each object to a 2d canvas.
	glEnable(GL_DEPTH_TEST);

	// This is our render loop. As long as our window remains open (ESC is not pressed), we'll continue to render things.
	while (!glfwWindowShouldClose(window))
	{

		// Clear the entire buffer with our green color (sets the background to be green).
		glClearBufferfv(GL_COLOR, 0, clear_color);
		glClearBufferfv(GL_DEPTH, 0, clear_depth);

		Keyframe k0, k1, res;
		float time = glfwGetTime();
		float f = getTimeFraction(time, 60.0);
		int num = getKeyframes(bKeyframes, f, k0, k1);
		bool ret = interpolateKeyframe(f, k0, k1, res);
		
		//Sets the view matrix to look at b if we choose to follow it.
		if (follow_b) rotated_view = glm::lookAt(glm::vec3(0.0f, 2.0f, 7.0f) , res._p, glm::vec3(0.0f, 1.0f, 0.0f));
		else rotated_view = glm::lookAt(glm::vec3(0.0f, 2.0f, 7.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		// this changes the camera location to origin to render the skybox.
		/*if (follow_b) skybox_view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), res._p, glm::vec3(0.0f, 1.0f, 0.0f));
		else skybox_view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));*/

		//this renders the skybox.
		/*glUseProgram(skybox);
		glUniformMatrix4fv(skyboxViewMatrixLocation, 1, GL_FALSE, &skybox_view[0][0]);
		glUniformMatrix4fv(skyboxProjectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		renderSkybox(skytex, skybox_view, projectionMatrix);*/
		
		// Enable the shader program
		glUseProgram(program);
		glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &rotated_view[0][0]); // send the view matrix to our shader
		inverseViewMatrix = glm::inverse(rotated_view);
		glUniformMatrix4fv(inverseViewLocation, 1, GL_FALSE, &inverseViewMatrix[0][0]);
		
		/*glUseProgram(picker);
		glUniformMatrix4fv(pickerViewMatrixLocation, 1, GL_FALSE, &rotated_view[0][0]); // send the view matrix to our shader*/
		
		select_index = getSelection(res);
		if (select_index == 1.0) {
			highlight_pc = true;
			highlight_b = false;
			if (mouse_release == true && follow_b == true) {
				follow_b = false;
				mouse_release = false;
			}
		}
		else if (select_index == 2.0) {
			highlight_b = true;
			highlight_pc = false;
			if (mouse_release == true && follow_b == false) {
				follow_b = true;
				mouse_release = false;
			}
		}
		else{
			highlight_b = false;
			highlight_pc = false;
			if (mouse_release == true) {
				follow_b = false;
				mouse_release = false;
			}
		}

		glClearBufferfv(GL_COLOR, 0, clear_color);
		glClearBufferfv(GL_DEPTH, 0, clear_depth);

		glUseProgram(program);
		
		renderPC(program);
		
		renderB(program, res);

		// disable the shader program
		glUseProgram(0);

		// Swap the buffers so that what we drew will appear on the screen.
		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	// Program clean up when the window gets closed.
	glDeleteVertexArrays(2, vaoID);
	glDeleteProgram(program);
	//glDeleteProgram(picker);
	//glDeleteProgram(skybox);
}

