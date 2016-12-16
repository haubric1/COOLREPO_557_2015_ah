#version 410 core                                                 
                                                                  
in vec3 pass_Color;
in vec2 vN;                                                 

uniform sampler2D tex;
uniform bool env_map;

out vec4 color;                                                    
void main(void)                                                   
{                                                                 
    if(env_map){
        vec4 tex_color = texture(tex, vN);
        color = 0.5 * tex_color + 0.5 * vec4(pass_Color, 1.0);
    }
    else{
        color = vec4(pass_Color, 1.0);
    }                            
}                                                                 