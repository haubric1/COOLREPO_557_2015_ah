#version 410 core                                                 

in vec2 pass_TexCoord; //this is the texture coord                                                                 
in vec3 pass_Color;                                               
out vec4 color;

uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform int texture_blend;
                                                   
void main(void)                                                   
{                                                                 
    // Thisfunction finds the color component for each texture coordinate.
    vec4 tex1_color = texture(tex1, pass_TexCoord);
    vec4 tex2_color = texture(tex2, pass_TexCoord);
    vec4 tex3_color = texture(tex3, pass_TexCoord);

    //This decides our blend of choice.
    /*switch(texture_blend) {
        case 0 : color = (pass_Color, 1.0) * tex_color;
                 break;
        case 1 : color = (pass_Color, 1.0) + tex_color;
                 break;
        default : color = (pass_Color, 1.0) * tex_color;                       
    }*/

    color = (pass_Color, 1.0) * tex3_color + tex1_color * tex2_color;
}