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

// this line tells the compiler to use the namespace std.
// Each object, command without a namespace is assumed to be part of std. 
using namespace std;
#include <algorithm>




static const string vs_string =
"#version 410 core																						\n"
"																										\n"
"uniform mat4 projectionMatrix;																			\n"
"uniform mat4 viewMatrix;																				\n"
"uniform mat4 modelMatrix;																				\n"
"uniform vec3 diffuse_color;																			\n"
"uniform vec3 ambient_color;																			\n"
"uniform vec3 specular_color;																			\n"
"uniform vec3 light_position;																			\n"
"uniform float diffuse_intensity;																		\n"
"uniform float ambient_intensity;																		\n"
"uniform float specular_intensity;																		\n"
"uniform float shininess;																				\n"
"in vec3 in_Position;																					\n"
"in vec3 in_Normal;																						\n"
"out vec3 pass_Color;																					\n"
"																										\n"
"void main(void)																						\n"
"{																										\n"
"	vec3 normal = normalize(in_Normal);																	\n"
"	vec4 transformedNormal = normalize(transpose(inverse(modelMatrixBox)) * vec4( normal, 1.0 ));		\n"
"	vec4 surfacePostion = modelMatrixBox * vec4(in_Position, 1.0);										\n"
"																										\n"	                                                                                                            
"	vec4 surface_to_light = normalize(light_position - surfacePostion);									\n"
"	if (light_position.w == 0.0) {																		\n"
"		surface_to_light = normalize(light_position);													\n"
"	}																									\n"
"																										\n"
"	// Diffuse color																					\n"
"	float diffuse_coefficient = max(dot(transformedNormal, surface_to_light), 0.0);						\n"
"	vec3 out_diffuse_color = diffuse_color  * diffuse_coefficient * diffuse_intensity;					\n"
"																										\n"
"	// Ambient color                                                                                    \n"     
"	vec3 out_ambient_color = vec3(ambient_color) * ambient_intensity;									\n"
"																										\n"
"	// Specular color                                                                                   \n"
"	vec3 incidenceVector = -surface_to_light.xyz;														\n"
"	vec3 reflectionVector = reflect(incidenceVector, normal);											\n"
"	vec3 cameraPosition = vec3(inverseViewMatrix[3][0], inverseViewMatrix[3][1], inverseViewMatrix[3][2]); \n"
"	vec3 surfaceToCamera = normalize(cameraPosition - surfacePostion.xyz);								\n"
"	float cosAngle = max(dot(surfaceToCamera, reflectionVector), 0.0);									\n"
"	float specular_coefficient = pow(cosAngle, shininess);												\n"
"	vec3 out_specular_color = specular_color * specular_coefficient * specular_intensity;				\n"
"																										\n"
"																										\n"
"	//attenuation																						\n"
"	float distanceToLight = length(light_position.xyz - surfacePostion.xyz);							\n"
"	float attenuation = 1.0 / (1.0 + attenuationCoefficient * pow(distanceToLight, 2));					\n"
"																										\n																								\n"
"																										\n"
"																										\n"
"	// Calculate the linear color																		\n"
"	vec3 linearColor = out_ambient_color + attenuation * (out_diffuse_color + out_specular_color);		\n"
"																										\n"
"	// Gamma correction																					\n"
"	vec3 gamma = vec3(1.0 / 2.2);																		\n"
"	vec3 finalColor = pow(linearColor, gamma);															\n"
"																										\n"
"	// Pass the color																					\n"
"	pass_Color = finalColor;																			\n"
"																										\n"
"	// Passes the projected position to the fragment shader / rasterization process.					\n"
"	gl_Position = projectionMatrixBox * viewMatrixBox * modelMatrixBox * vec4(in_Position, 1.0);		\n"
"}																										\n";

// Fragment shader source code. This determines the colors in the fragment generated in the shader pipeline. In this case, it colors the inside of our triangle specified by our vertex shader.
static const string fs_string  =
"#version 410 core                                                 \n"
"                                                                  \n"
"in vec3 pass_Color;                                                 \n"
"out vec4 color;                                                    \n"
"void main(void)                                                   \n"
"{                                                                 \n"
"    color = vec4(pass_Color, 1.0);                               \n"
"}                                                                 \n";




/// Camera control matrices
glm::mat4 projectionMatrix; // Store the projection matrix
glm::mat4 viewMatrix; // Store the view matrix
glm::mat4 modelMatrix; // Store the model matrix




// The handle to the window object
GLFWwindow*         window;


// Define some of the global variables we're using for this sample
GLuint program;

/**
This computes the number of array elements necessary to store the coordinates for the sphere
@param rows - the number of rows for the sphere
@param segments - the number of segments for the sphere
@return the number of array elements.
**/
int NumVec3ArrayElements(const int rows, const int segments)
{
	return  ((segments * 2) + 1) * 6 * rows;
}


/**
The function creates the vertices for a sphere. Note, the sphere must be rendered as a GL_TRIANGLE_STRIP to obtain
a complete surface model. The outcome are two arrays with points.

The number of elements you need for your array is

N = (segments * 2 + 1) * 6 * rows
e.g. with 10 rows and 10 segments
N = (10*2 + 1) * 6* 10 = 1260 elememts -> float vertices[1260]l float normals[1260];
which results in 1260/3 = 420 vertices

@oaram rows - the number of vertex rows.
@param segments - the number of segments per row.
@param center - a pointer to a array with 3 elements of the origin [x, y, z]
@param r - the radius of the sphere
@param spherePoints - pointer to a vector that can contain the vertices. The vector must already been initialized
@param normals - pointer to a vector that can contain the normals. The vector must already been initialized
**/
int Make_Sphere(const int rows, const int segments, const float *center, const float r, float* spherePoints, float* normals)
{


	const float PI = 3.141592653589793238462643383279502884197f;

	int current_size = 0;
	for (float theta = 0.; theta < PI; theta += PI / float(rows)) // Elevation [0, PI]
	{
		//double theta = 1.57;
		float theta2 = theta + PI / float(rows);
		int count = 0;
		int count_row = 0;

		// Iterate through phi, theta then convert r,theta,phi to  XYZ
		for (float phi = 0.; phi < 2 * PI + PI / float(segments); phi += PI / float(segments)) // Azimuth [0, 2PI]
		{
			int index = current_size + count;

			spherePoints[index] = r * cos(phi) * sin(theta) + center[0];
			spherePoints[index + 1] = r * sin(phi) * sin(theta) + center[1];
			spherePoints[index + 2] = r            * cos(theta) + center[2];
			count += 3;

			spherePoints[index + 3] = r * cos(phi) * sin(theta2) + center[0];
			spherePoints[index + 4] = r * sin(phi) * sin(theta2) + center[1];
			spherePoints[index + 5] = r            * cos(theta2) + center[2];
			count += 3;

			normals[index] = cos(phi) * sin(theta);
			normals[index + 1] = sin(phi) * sin(theta);
			normals[index + 2] = cos(theta);

			normals[index + 3] = cos(phi) * sin(theta2);
			normals[index + 4] = sin(phi) * sin(theta2);
			normals[index + 5] = cos(theta2);

		}
		if (count_row == 0) count_row = count;

		current_size += count;
	}
	return current_size / 3;
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Fill this functions with your model code.

// USE THESE vertex array objects to define your objects
unsigned int vaoID[2];

unsigned int vboID[4];

/*!
 ADD YOUR CODE TO CREATE THE TRIANGLE STRIP MODEL TO THIS FUNCTION
 */
unsigned int createSphereModel(void)
{
	// use the vertex array object vaoID[0] for this model representation
	// Vertex and Color arrays
	GLfloat center[3] = { 0.0f, 0.0f, 0.0f };

	GLfloat radius = 10;

	int segments = 20;
	int rows = 20;

	int N = NumVec3ArrayElements(rows, segments);

	GLfloat points[4920];
	GLfloat normals[4920];
	GLfloat colors[4920];

	int num_vertices = Make_Sphere(rows, segments, center, radius, points, normals);

	for (int i = 0; i < 4920/3; ++i) {
		colors[3 * i] = 1.0f;
		colors[(3 * i) + 1] = 1.0f;
		colors[(3 * i) + 2] = 1.0f;
	};


    //TODO:
	
	glGenVertexArrays(1, &vaoID[0]); //creating vertex array objects for the triangle strips model
	glBindVertexArray(vaoID[0]); //binding it!

	glGenBuffers(2, vboID); //generate our vertex buffer object

	//vertices!
	glBindBuffer(GL_ARRAY_BUFFER, vboID[0]); //bind our VBO
	glBufferData(GL_ARRAY_BUFFER, 4920 * sizeof(GLfloat), points, GL_STATIC_DRAW);

	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);//disable the vertex array

	//normals!
	glBindBuffer(GL_ARRAY_BUFFER, vboID[1]); //bind our VBO
	glBufferData(GL_ARRAY_BUFFER, 4920 * sizeof(GLfloat), normals, GL_STATIC_DRAW);

	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);//disable the vertex array

	glBindVertexArray(0);
    
    return 1;
}

/*!
 ADD YOUR CODE TO RENDER THE TRIANGLE STRIP MODEL TO THIS FUNCTION
 */
void renderSphereModel(void)
{

    // Bind the buffer and switch it to an active buffer
    glBindVertexArray(vaoID[0]);
        
	// HERE: THIS CAUSES AN ERROR BECAUSE I DO NOT KNOW HOW MANY TRIANGLES / VERTICES YOU HAVE.
	// COMPLETE THE LINE
    // Draw the triangles
    glDrawArrays(GL_TRIANGLE_STRIP, 0 , 4920);

    // Unbind our Vertex Array Object
    glBindVertexArray(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*!
 This function creates the two models
 */
void setupScene(void) {
    
    createSphereModel();
    
}




int main(int argc, const char * argv[])
{
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// Init glfw, create a window, and init glew
    
    // Init the GLFW Window
    window = initWindow();
    
    
    // Init the glew api
    initGlew();
    
	// Prepares some defaults
	CoordSystemRenderer* coordinate_system_renderer = new CoordSystemRenderer(10.0);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// The Shader Program starts here
    
    // Vertex shader source code. This draws the vertices in our window. We have 3 vertices since we're drawing an triangle.
    // Each vertex is represented by a vector of size 4 (x, y, z, w) coordinates.
    static const string vertex_code = vs_string;
    static const char * vs_source = vertex_code.c_str();
    
    // Fragment shader source code. This determines the colors in the fragment generated in the shader pipeline. In this case, it colors the inside of our triangle specified by our vertex shader.
    static const string fragment_code = fs_string;
    static const char * fs_source = fragment_code.c_str();
    
    // This next section we'll generate the OpenGL program and attach the shaders to it so that we can render our triangle.
    program = glCreateProgram();
    
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
    
    // We'll specify that we want to use this program that we've attached the shaders to.
    glUseProgram(program);
    
    //// The Shader Program ends here
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    
    /// IGNORE THE NEXT PART OF THIS CODE
    /// IGNORE THE NEXT PART OF THIS CODE
    /// IGNORE THE NEXT PART OF THIS CODE
    // It controls the virtual camera
    
    // Set up our white background color
    static const GLfloat clear_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    static const GLfloat clear_depth[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    
    projectionMatrix = glm::perspective(1.1f, (float)800 / (float)600, 0.1f, 200.f);
    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)); // Create our model matrix which will halve the size of our model
    viewMatrix = glm::lookAt(glm::vec3(1.0f, 0.0f, 100.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    int projectionMatrixLocation = glGetUniformLocation(program, "projectionMatrix"); // Get the location of our projection matrix in the shader
    int viewMatrixLocation = glGetUniformLocation(program, "viewMatrix"); // Get the location of our view matrix in the shader
    int modelMatrixLocation = glGetUniformLocation(program, "modelMatrix"); // Get the location of our model matrix in the shader
    
    
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]); // Send our projection matrix to the shader
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]); // Send our view matrix to the shader
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]); // Send our model matrix to the shader

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Material
	
	//setting our diffuse, ambient, and specular colors.
	glm::vec3 diffuse_material = glm::vec3(1.0, 0.0, 0.0);
	glm::vec3 ambient_material = glm::vec3(1.0, 0.5, 0.0);
	glm::vec3 specular_material = glm::vec3(1.0, 1.0, 1.0);

	//setting the position of our light.
	glm::vec3 light_position = glm::vec3(-34.0, -5.0, 0.0);

	//setting the shininess of our object.
	float shininess = 1.0f;

	//setting the intensities of our lights.
	float diffuse_intensity = 0.5f;
	float ambient_intensity = 0.0f;
	float specular_intensity = 0.0f;

	//getting pointers to our uniform variables in our shader
	int ambientColorPos = glGetUniformLocation(program, "ambient_color");
	int diffuseColorPos = glGetUniformLocation(program, "diffuse_color");
	int specularColorPos = glGetUniformLocation(program, "specular_color");
	int lightPosPos = glGetUniformLocation(program, "light_position");
	int diffuseIntensityPos = glGetUniformLocation(program, "diffuse_intensity");
	int ambientIntensityPos = glGetUniformLocation(program, "ambient_intensity");
	int specularIntensityPos = glGetUniformLocation(program, "specular_intensity");
	int shininessIdx = glGetUniformLocation(program, "shininess");


	// Send the material to your shader program
	glUniform3fv(ambientColorPos, 1, &ambient_material[0]);
	glUniform3fv(diffuseColorPos, 1, &diffuse_material[0]);
	glUniform3fv(specularColorPos, 1, &specular_material[0]);
	glUniform3fv(lightPosPos, 1, &light_position[0]);
	glUniform1f(diffuseIntensityPos, diffuse_intensity);
	glUniform1f(ambientIntensityPos, ambient_intensity);
	glUniform1f(specularIntensityPos, specular_intensity);
	glUniform1f(shininessIdx, shininess);
    

	glBindAttribLocation(program, 0, "in_Position");
	glBindAttribLocation(program, 1, "in_Normal");

	 //// The Shader Program ends here
    //// START TO READ AGAIN
    //// START TO READ AGAIN
    //// START TO READ AGAIN
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    
    // this creates the scene
    setupScene();
    
    int i=0;

    // Enable depth test
    // ignore this line, it allows us to keep the distance value after we proejct each object to a 2d canvas.
    glEnable(GL_DEPTH_TEST);
    
    // This is our render loop. As long as our window remains open (ESC is not pressed), we'll continue to render things.
    while(!glfwWindowShouldClose(window))
    {
        
        // Clear the entire buffer with our green color (sets the background to be green).
        glClearBufferfv(GL_COLOR , 0, clear_color);
        glClearBufferfv(GL_DEPTH , 0, clear_depth);
        

		coordinate_system_renderer->draw();
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //// This generate the object
        // Enable the shader program
        glUseProgram(program);
        
        // this changes the camera location
        glm::mat4 rotated_view = viewMatrix * GetRotationMatrix();
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &rotated_view[0][0]); // send the view matrix to our shader
        

        // This moves the model far to the right
        modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(34.00f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]); // Send our model matrix to the shader
        

		// Render the first sphere!
        renderSphereModel();

		// This moves the model to the right
		modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(12.00f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]); // Send our model matrix to the shader
        
		// Render a second sphere!
		renderSphereModel();

        // This moves the model to the left
        modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-12.00f, -0.0f, 0.0f));
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]); // Send our model matrix to the shader
        
		// Render a third sphere!
        renderSphereModel();

		//This moves the model far to the left.
		modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-34.00f, -0.0f, 0.0f));
		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]); // Send our model matrix to the shader

		// Render the fourth, and final, sphere!
		renderSphereModel();
        
        
		// disable the shader program
        glUseProgram(0);


        //// This generate the object
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        // Swap the buffers so that what we drew will appear on the screen.
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }
    
	// delete the coordinate system object
	delete coordinate_system_renderer;

    // Program clean up when the window gets closed.
    glDeleteVertexArrays(2, vaoID);
    glDeleteProgram(program);
}

