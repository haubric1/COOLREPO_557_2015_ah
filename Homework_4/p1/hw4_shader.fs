#version 410 core                                                 

in vec2 pass_TexCoord; //this is the texture coord                                                                 
in vec3 pass_Color;                                               
out vec4 color;

uniform sampler2D tex;
uniform int texture_blend;
                                                   
void main(void)                                                   
{                                                                 
    // Thisfunction finds the color component for each texture coordinate.
    vec4 tex_color = texture(tex, pass_TexCoord);

    //This decides our blend of choice.
    /*switch(texture_blend) {
        case 0 : color = (pass_Color, 1.0) * tex_color;
                 break;
        case 1 : color = (pass_Color, 1.0) + tex_color;
                 break;
        default : color = (pass_Color, 1.0) * tex_color;                       
    }*/

    color = tex_color * (pass_Color, 1.0);
}