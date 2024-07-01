#version 330 core

in vec3 frag_surface_normal_color;

uniform sampler2D shadow_buffer_tex;

uniform mat4 model;
uniform vec3 view_pos;
uniform mat4 projection;

uniform vec3 light_position;

in float red;
in float green;
in float blue;
out vec4 color;

in vec3 frag_position;
in vec3 frag_normal;
in vec4 shadow_coord;

uniform uint window_width;
uniform uint window_height;
uniform uint shadow_mapping_effect;

uniform vec3 shape_color;
uniform vec3 sun_col;

uniform uint shadow_buffer_tex_size;

void main(){
   float rand_val = sin(frag_position.x*frag_position.z/10)+cos( (0.13-frag_position.x*frag_position.y) / 10 );
    // Luz ambiente
   vec3 ambient = 0.5 * sun_col;

   // Luz difusa
   vec3 light_dir = normalize(light_position-frag_position);
   float diff = max(dot(frag_normal, light_dir), 0.0);
   vec3 diffuse = diff * sun_col;

   // // luz specular
   vec3 view_dir = normalize(view_pos - frag_position);
   vec3 reflect_dir = reflect(-light_dir, frag_normal);
   float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
   vec3 specular = spec * sun_col;

   // Mezcla de luces
   vec3 lighting = specular + ambient + diffuse;

   vec3 base_color = vec3(0.1 + rand_val/6, 0.4, 0.1);
   color.rgb = lighting * base_color;
   color = vec4(color.rgb, 1.0);
}
