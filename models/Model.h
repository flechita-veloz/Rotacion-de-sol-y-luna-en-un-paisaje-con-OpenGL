#include <GLFW/glfw3.h>
#include <iostream>
#include "common/texture.hpp"
#include "common/objloader.hpp"
#include "../tree/shader_helper.h"
#include "../../sphere/src/glad/include/glad/glad.h"

using namespace std;

class Model {
public:
    GLuint shader, texture;
    GLuint MVP_id, texture_id;
    vector<glm::vec3> vertices;
    vector<glm::vec2> uvs;
    vector<glm::vec3> normals;
    GLuint VertexArrayID;
    GLuint vertexbuffer, uvbuffer, normalsbuffer;
    glm::mat4 projection, view, transform;
    GLuint view_pos_id, light_position_id, lightColor_id, transform_id;
    GLfloat cameraPos[3], lightPosition[3], lightColor[3];
    string name;

    void loadShaders(const char *vshader, const char *fshader) {
        this->shader = load_shaders(vshader, fshader);
        MVP_id = glGetUniformLocation(shader, "MVP");
        transform_id = glGetUniformLocation(shader, "transform");
    }
    
    void setName(string name){
        this->name = name;
    }

    void loadTexture(const char *path) {
        this->texture = loadDDS(path);
        this->texture_id = glGetUniformLocation(shader, "myTextureSampler");
    }

    void loadModel(const char *path) {
        bool res = loadOBJ(path, vertices, uvs, normals);
    }

    void init() {
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);

        // Vertex buffer
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

        // UV buffer
        glGenBuffers(1, &uvbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

        // Normals buffer
        glGenBuffers(1, &normalsbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

        glUseProgram(shader);
        view_pos_id = glGetUniformLocation(shader, "view_pos");
        light_position_id = glGetUniformLocation(shader, "light_position");
        lightColor_id = glGetUniformLocation(shader, "lightColor");

        transform = glm::mat4(1.0);
    }

    void setView(glm::mat4 view) {
        this->view = view;
    }

    void setProjection(glm::mat4 projection) {
        this->projection = projection;
    }

    void setCameraPos(glm::vec3 cameraPos) {
        this->cameraPos[0] = cameraPos[0];
        this->cameraPos[1] = cameraPos[1];
        this->cameraPos[2] = cameraPos[2];
    }

    void setLightPos(glm::vec3 lightPosition) {
        this->lightPosition[0] = lightPosition[0];
        this->lightPosition[1] = lightPosition[1];
        this->lightPosition[2] = lightPosition[2];
    }

    void setLightColor(glm::vec3 lightColor) {
        this->lightColor[0] = lightColor[0];
        this->lightColor[1] = lightColor[1];
        this->lightColor[2] = lightColor[2];
    }

    void draw() {
        glUseProgram(shader);
        glBindVertexArray(VertexArrayID);

        glUniform3fv(view_pos_id, 1, cameraPos);
        glUniform3fv(light_position_id, 1, lightPosition);
        glUniform3fv(lightColor_id, 1, lightColor);
        glUniformMatrix4fv(transform_id, 1, GL_FALSE, &transform[0][0]);

        glm::mat4 MVP = projection * view * transform;
        glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &MVP[0][0]);

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(texture_id, 0);

        // Vertex attribute
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
            0,                  // attribute location
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        // UV attribute
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
            1,                  // attribute location
            2,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        // Normals attribute
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer);
        glVertexAttribPointer(
            2,                  // attribute location
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        // Draw the model
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void rotate(float angle, glm::vec3 axis) {
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
        transform = rotationMatrix * transform;
    }

    void translate(glm::vec3 translation) {
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);
        transform = translationMatrix * transform;
    }

    void scale(glm::vec3 scaling) {
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scaling);
        transform = scaleMatrix * transform;
    }

    void resetTransform() {
        transform = glm::mat4(1.0f);
    }

    void setTransform(const glm::mat4& newTransform) {
        transform = newTransform;
    }
};
