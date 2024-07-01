#version 410 core
out vec4 FragColor;
uniform float intensity;
in GS_OUT {
	vec2 textCoord;
    float colorVariation;
} fs_in;

uniform sampler2D u_textgrass;

void main(){
    
    vec4 color = texture(u_textgrass, fs_in.textCoord);
    
    if (color.a < 0.25 ) discard;
    color.xyz = mix(color.xyz, 0.5*color.xyz, fs_in.colorVariation);

    vec3 new_color = color.rgb * intensity;
	FragColor = vec4(new_color, 1.0);
}
