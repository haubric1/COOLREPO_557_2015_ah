#include <iostream>
#include <cmath>

using namespace std;


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
    for (float theta = 0.; theta < PI; theta += PI/float(rows)) // Elevation [0, PI]
    {
        //double theta = 1.57;
        float theta2 = theta + PI/float(rows);
        int count = 0;
        int count_row = 0;
        
        // Iterate through phi, theta then convert r,theta,phi to  XYZ
        for (float phi = 0.; phi < 2*PI + PI/float(segments) ; phi += PI/float(segments)) // Azimuth [0, 2PI]
        {
            int index = current_size + count;

            spherePoints[index] 	 = r * cos(phi) * sin(theta) + center[0];
            spherePoints[index + 1] = r * sin(phi) * sin(theta) + center[1];
            spherePoints[index + 2] = r            * cos(theta) + center[2];
			count+=3;
            
            spherePoints[index + 3] = r * cos(phi) * sin(theta2) + center[0];
            spherePoints[index + 4] = r * sin(phi) * sin(theta2) + center[1];
            spherePoints[index + 5] = r            * cos(theta2) + center[2];
            count+=3;
            
            normals[index]     =  cos(phi) * sin(theta);
            normals[index + 1] =  sin(phi) * sin(theta);
            normals[index + 2] =  cos(theta);
            
            normals[index + 3]  =  cos(phi) * sin(theta2);
            normals[index + 4]  =  sin(phi) * sin(theta2);
            normals[index + 5]  =  cos(theta2);

        }
        if(count_row == 0) count_row = count;

		current_size += count;
    }
    return current_size/3;
}




int __main(int argc, char *argv[]) {

	// Set the center and the radius
	float center[3] = {0.0, 0.0, 0.0};
	float radius = 10.0;

	
	// Set the number of segment elements and rows of the sphere. 
	// This will affect the number of triangles in the model
	int segments = 10;
	int rows = 10;

	
	// Compute the number of array elements necessary to store all coordinates.
	int  N = NumVec3ArrayElements(rows, segments);


	// create memory for the vertices and the normals. 
	float* points = new float[N];
	float* normals = new float[N];
	

	// Create the points and the normal vectors
	int number_vertices = Make_Sphere(rows, segments, center, radius, points, normals);
	
	
	std::cout << number_vertices;
	return 1;
}