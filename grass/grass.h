#include "stdio.h"
#include "stdlib.h"
#include "vector"
#include "iostream"
#include "chrono"
#include "thread"

// Include GLAD
#include "../sphere/src/glad/include/glad/glad.h"

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Include shader
#include "common/Shader.hpp"
#include "common/stb_image.hpp"
#define STB_IMAGE_IMPLEMENTATION

// to get path || include shader
#include <unistd.h>
#define GetCurrentDir getcwd

#define STB_IMAGE_IMPLEMENTATION

using namespace std;
// 3456 por 2234
// cambiar 
const unsigned int SCR_WIDTH = 3456;
const unsigned int SCR_HEIGHT = 2234;
// const unsigned int SCR_WIDTH = 1920;
// const unsigned int SCR_HEIGHT = 1080;

// camera
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.7f, 8.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw   = -90.0f;   // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0;
float fov   =  45.0f;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;
GLfloat intensityGrass; // INTENSIDAD DEL LA LUZ PARA EL PASTO

unsigned int loadTextureFromFile(const char *path)
{
    //std::string filename = std::string(path);
    std::string filename = std::string(path);//directory + '/' + filename;
    
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);  
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    stbi_set_flip_vertically_on_load(false);  
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
class Grass {
public:
    Grass(const std::string& shaderPath, const std::string& texturePath1, const std::string& texturePath2) {
        // Initialize shader
        shaderID = load_shaders2((shaderPath + "grass.vs.glsl").c_str(),
                                 (shaderPath + "grass.fs.glsl").c_str(),
                                 (shaderPath + "grass.gs.glsl").c_str());
        glUseProgram(shaderID);
        
        // Initialize projection matrix
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "u_projection"), 1, GL_FALSE, &projection[0][0]);

        // Initialize view matrix
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "u_view"), 1, GL_FALSE, &view[0][0]);
        
        // Generate positions 
        generatePositions();

        // Setup buffers
        setupBuffers();
        
        // Load textures
        texture1 = loadTextureFromFile(texturePath1.c_str());
        glUniform1i(glGetUniformLocation(shaderID, "u_texture1"), 0);
        texture2 = loadTextureFromFile(texturePath2.c_str());
        glUniform1i(glGetUniformLocation(shaderID, "u_wind"), 1);
    }

    void draw() {
        glUseProgram(shaderID);
        
        // Bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // Update view matrix
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "u_view"), 1, GL_FALSE, &view[0][0]);
        glUniform3fv(glGetUniformLocation(shaderID, "u_cameraPosition"), 1, &cameraPos[0]);
        glUniform1f(glGetUniformLocation(shaderID, "u_time"), glfwGetTime());
        glUniform1f(glGetUniformLocation(shaderID, "intensity"), intensityGrass);

        // Draw
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, positions.size());
    }

    ~Grass() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteProgram(shaderID);
        glDeleteTextures(1, &texture1);
        glDeleteTextures(1, &texture2);
    }

private:
    GLuint shaderID;
    GLuint VBO, VAO;
    GLuint texture1, texture2;
    std::vector<glm::vec3> positions;

    void generatePositions() {
        srand(time(NULL));
        for (float x = -50.0f; x < 50.0f; x += 0.1f)
            for (float z = -50.0f; z < 50.0f; z += 0.1f) {
                int randNumberX = rand() % 10 + 1;
                int randNumberZ = rand() % 10 + 1;
                positions.push_back(glm::vec3(x + (float)randNumberX / 50.0f, 0, z + (float)randNumberZ / 50.0f));
            }
    }

    void setupBuffers() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
};

