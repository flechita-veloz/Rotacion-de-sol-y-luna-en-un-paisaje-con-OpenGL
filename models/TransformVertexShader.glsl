#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 normals;

// Output data; will be interpolated for each fragment.
out vec2 UV;
out vec3 frag_normal;
out vec3 frag_position;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 transform;

void main(){
    // Output position of the vertex, in clip space: MVP * position
    gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);
    frag_position = vec3(transform * vec4(vertexPosition_modelspace, 1.0));

    // UV of the vertex. No special space for this one.
    UV = vertexUV;

    // Transform the normal with the inverse transpose of the model matrix.
    mat3 normalMatrix = mat3(transpose(inverse(transform)));
    frag_normal = normalize(normalMatrix * normals);
}
