#version 410 core
layout (location = 0) in vec3 aPos;
in vec3 normals;
// out VS_OUT {} vs_out;

void main() {
	gl_Position = vec4(aPos, 1.0); 
	// calculo de normales 
   // mat3 normalMat = mat3(global_model);
   // mat3 normalMat = mat3(global_model);
   // normalMat = transpose(inverse(normalMat));
   // vec3 normal_transformed = normalize(normalMat * surface_normal);
   // frag_normal = normal_transformed;
}