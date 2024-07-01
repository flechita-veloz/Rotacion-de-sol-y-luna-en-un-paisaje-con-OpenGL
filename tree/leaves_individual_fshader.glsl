#version 330 core

in vec3 frag_position;

uniform vec3 view_pos;
uniform vec3 light_position;
uniform vec3 sun_col;

in vec2 uv_frag;
in vec3 frag_normal;
in vec3 normals;

out vec4 color;
in float frag_diffuse_light;

in float frag_relative_ground_pos;

void main(){

   float shadow = 1.0;
   float distance_to_middle = distance(uv_frag, vec2(0.5, 0.5));

   if(distance_to_middle > 0.4 ){
      discard;
   }

   float rand_val = sin(frag_position.x*frag_position.z/10)+cos( (0.13-frag_position.x*frag_position.y) / 10 );

   // Luz ambiente
   vec3 ambient = 0.5 * sun_col;

   // Luz difusa
   vec3 light_dir = normalize(light_position - frag_position);
   float diff = max(dot(frag_normal, light_dir), 0.0);
   vec3 diffuse = diff * sun_col;

   // luz specular
   float specularStrength = 0.5; 
   vec3 view_dir = normalize(view_pos - frag_position);
   vec3 reflect_dir = reflect(-light_dir, frag_normal);
   float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
   vec3 specular = specularStrength * spec * sun_col;

   // Mezcla de luces
   vec3 lighting = specular + ambient + diffuse;

   float reverse_dist_to_middle = 1.0 - distance_to_middle;

   vec3 base_color = vec3(0.1 + rand_val/6, 0.4, 0.1);
   color.rgb = lighting * base_color * reverse_dist_to_middle;
   color = vec4(color.rgb * shadow, 1.0);
}
