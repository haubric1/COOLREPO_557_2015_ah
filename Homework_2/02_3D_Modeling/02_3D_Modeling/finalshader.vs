#version 410 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
																	
uniform vec3 light_position;										
uniform vec3 ambient_color;										
uniform vec3 diffuse_color;										
uniform vec3 specular_color;										
uniform vec3 cone_direction;										
																	
uniform float ambient_intensity;									
uniform float diffuse_intensity;									
uniform float specular_intensity;									
uniform float shininess;											
uniform float cone_angle;											
																	
uniform float attenuation_coefficient;								
																	
in vec3 in_Position;                                               
in vec3 in_Normal;                                                  
																	
out vec3 pass_Color;                                               
                                                                  
void main(void)                                                   
{                                                                 
	vec3 normal = normalize(in_Normal);															 
	vec4 transformedNormal = normalize(transpose(inverse(modelMatrix)) * vec4( normal, 1.0 ));	 
	vec4 surfacePosition = viewMatrix * modelMatrix * vec4( in_Position , 1.0 );				 
																								 
	vec4 surface_to_light = normalize( vec4(light_position,1.0) - surfacePosition);				 
																								 
	// Diffuse Color																			 
	float diffuse_coefficient = max( dot(transformedNormal, surface_to_light), 0.0);			 
	vec3 out_diffuse_color = diffuse_color * diffuse_coefficient * diffuse_intensity;			 
																								 
	// Ambient Color																			 
	vec3 out_ambient_color = ambient_color * ambient_intensity;									 
																								 
	// Specular Color																			 
	vec3 incidenceVector = -surface_to_light.xyz;												 
	vec3 reflectionVector = reflect(incidenceVector, transformedNormal.xyz);					 
	vec3 cameraPosition = vec3( -viewMatrix[3][0], -viewMatrix[3][1], -viewMatrix[3][2]);		 
	vec3 surfaceToCamera = normalize(cameraPosition - surfacePosition.xyz);						 
	float cosAngle = max( dot(surfaceToCamera, reflectionVector), 0.0);							 
	float specular_coefficient = pow(cosAngle, shininess);										 
	vec3 out_specular_color = specular_color * specular_coefficient * specular_intensity;		 
																								 
	// Attenuation																				 
	float distance_to_light = length(light_position.xyz - surfacePosition.xyz);					 
	float attenuation = 1.0 / (1.0 + attenuation_coefficient * pow(distance_to_light, 2));		 
																								 
	// Spotlight																				 
	vec3 cone_direction_norm = normalize(cone_direction);										 
	vec3 ray_direction = -surface_to_light.xyz;													 
	float light_to_surface_angle = degrees(acos(dot(ray_direction, cone_direction_norm)));		 
	if(light_to_surface_angle > cone_angle){													 
		attenuation = 0.0;																		 
	}																							 
																								 
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);			 
	vec3 linear_Color = vec3(out_ambient_color + attenuation * (out_diffuse_color + out_specular_color));				 
	vec3 gamma = vec3(1.0/2.2);																	 
	vec3 final_Color = pow(linear_Color, gamma);												 
	pass_Color = final_Color;																	 
}																								 