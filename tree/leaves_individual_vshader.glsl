#version 330 core

in vec3 position;
in vec3 surface_normal;
in vec2 uv;
in mat4 model_mat;
in vec3 raw_position;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 light_position;
uniform vec3 camera_position;

uniform vec2 wind_offset;
uniform sampler2D tex_wind;

uniform vec3 sun_dir;

out vec3 frag_position;
out vec2 uv_frag;
out vec3 frag_normal;

out float frag_diffuse_light;
out float frag_relative_ground_pos;

void main(){
   mat4 global_model = model * model_mat;
   vec3 new_pos_tex = vec3(global_model * vec4(position, 1.0));

   vec2 relative_tex_pos = vec2(new_pos_tex.x/40+0.5, new_pos_tex.z/40+0.5);

   float wind_x = texture(tex_wind, (relative_tex_pos+wind_offset) ).r;
   float wind_z = texture(tex_wind, (relative_tex_pos+vec2(0.5, 0.5)+wind_offset) ).r;

   //rotation around x axis
   mat4 rot_mat_x = mat4(1, 0, 0, 0,
                         0, cos(wind_x), -sin(wind_x), 0,
                         0, sin(wind_x), cos(wind_x), 0,
                         0, 0, 0, 1);

   //rotation around z axis
   mat4 rot_mat_z = mat4(cos(wind_z), -sin(wind_z), 0, 0,
                          sin(wind_z), cos(wind_x), 0, 0,
                          0, 0, 0, 0,
                          0, 0, 0, 1);

   vec3 new_pos = vec3(global_model*rot_mat_x*rot_mat_z*vec4(position, 1.0));
   // vec3 new_pos = vec3(global_model * vec4(position, 1.0));
   float sun_intensity = dot(-sun_dir, vec3(0, 1, 0));

   sun_intensity = clamp(sun_intensity, 0.2, 1);

   float lum = 0.8 * sun_intensity;
   frag_diffuse_light = lum;

   frag_relative_ground_pos = raw_position.y;

   gl_Position = projection*view*vec4(new_pos, 1.0);

   frag_position = new_pos;
   uv_frag = uv;

   // calculo de normales 
   mat3 normalMat = mat3(global_model*rot_mat_x*rot_mat_z);
   
   // mat3 normalMat = mat3(global_model);
   normalMat = transpose(inverse(normalMat));
   vec3 normal_transformed = normalize(normalMat * surface_normal);
   // // Ajustar la orientación de la normal según la vista de la cámara
   if (dot(normal_transformed, new_pos - camera_position) < 0.0) {
      normal_transformed = -normal_transformed;
   }
   frag_normal = normal_transformed;
}
