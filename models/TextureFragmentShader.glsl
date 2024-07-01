#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;

// Output data
out vec3 color;
in vec3 frag_normal;
in vec3 frag_position;

uniform vec3 light_position;
uniform vec3 view_pos;
uniform vec3 lightColor;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

void main(){
	// Luz ambiente
	vec3 ambient = 0.5 * lightColor;

	// Luz difusa
	vec3 light_dir = normalize(light_position - frag_position);
	float diff = max(dot(frag_normal, light_dir), 0.0);
	vec3 diffuse = diff * lightColor;

	// luz specular
	float specularStrength = 0.1;
	vec3 view_dir = normalize(view_pos - frag_position);
	vec3 reflect_dir = reflect(-light_dir, frag_normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 8);
	vec3 specular = specularStrength * spec * lightColor;

	// // Mezcla de luces
   	vec3 lighting = ambient + diffuse + specular;

	// Output color = color of the texture at the specified UV
	color = lighting * texture( myTextureSampler, UV ).rgb;
}