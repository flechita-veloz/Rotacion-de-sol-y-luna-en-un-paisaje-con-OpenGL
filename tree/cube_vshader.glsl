#version 330 core

in vec3 position;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 clip_coord;
in vec3 surface_normal;

uniform mat4 shadow_matrix; //bias*P*V

out vec3 frag_surface_normal_color;
out vec3 frag_position;
out vec3 frag_normal;
out float red;
out float green;
out float blue;

out vec4 shadow_coord;
out float gl_ClipDistance[1];

void main(){
   gl_Position = projection*view*model*vec4(position, 1.0);
    //dont display anything under water, negative means outside clip plane
   gl_ClipDistance[0] = dot(model*vec4(position, 1.0), clip_coord);
   frag_position = vec3(model*vec4(position, 1.0));

   shadow_coord = shadow_matrix * model*vec4(position, 1.0);

   mat3 normalMat = mat3(model);
   normalMat = transpose(inverse(normalMat));
   vec3 normal_transformed = normalize(normalMat*surface_normal);
   frag_normal = normal_transformed;
}
