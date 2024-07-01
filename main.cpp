#include "sphere/src/drawSphere.cpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <chrono>
#include <GLFW/glfw3.h>
#include <GL/glext.h>
#include <IL/il.h>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //perspective
#include <vector>

#include "tree/shader_helper.h"
#include "tree/_cube/cube.h"
#include "tree/transform.h"
#include "tree/_quad_screen/quad_screen.h"
#include "tree/framebuffer.h"
#include "tree/drawable.h"
#include "tree/tree.h"
#include "grass.h"

#include "models/common/texture.hpp"
#include "models/common/objloader.hpp"
#include "Model.h"


// functions
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
std::string getCurrentWorkingDirectory ();

// ######################## ARBOL ##############################################
void init(int ind);
void display(float time_delta, Grass &pasto, double _frameTime, double currTime);
GLuint load_shader(char *path, GLenum shader_type);



// Define los colores para el día y la noche
vector<glm::vec3> background_color = {
    {0.05, 0.05, 0.2},  // noche
    {0.15, 0.15, 0.4},  // noche_claro
    {0.53, 0.81, 0.92}, // dia_claro
    {0.25, 0.45, 0.6}   // dia_oscuro
};
glm::vec3 memo1 = background_color[1], memo2; // colores utiles interpolar
glm::vec3 interpolatedColor;
glm::mat4 projection;

Framebuffer *framebuffer;

Cube cube_base;
Transform cube_base_transf;

// MODELOS
vector<Model> modelos;
// Model model;

// global variables
GLFWwindow* window;
GLFWwindow* window2;

vector<Tree*> arboles;
vector<Transform> transform_arboles;
Quad_screen quad_screen;
// glm::mat4x4 projection;
uint trunk_pid;
uint leaves_pid;

GLfloat light_position[3];
GLfloat camera_position[3] = {cameraPos[0], cameraPos[1], cameraPos[2]};
GLfloat camera_direction[3] = {-0.791659,-0.25702,0.55427};

float time_measured;

unsigned int effect_select;
// ######################## ARBOL ##############################################

float random(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

void crearArbol() {
    Tree* newTree =  new Tree;
    newTree->init(trunk_pid, leaves_pid);
    newTree->load();
    Transform newPosition;
    newPosition.translate({random(-5.0f, 5.0f), 0, random(-5.0f, 5.0f)});
    transform_arboles.push_back(newPosition);
    arboles.push_back(newTree);
    init(arboles.size() - 1);
}

void eliminarUltimoArbol() {
    int sz = arboles.size();
    if(sz != 0){
        delete arboles[sz - 1];
        arboles.pop_back();
        transform_arboles.pop_back();
    }
}


int verifyWindow(GLFWwindow* window){
    if (!window) {
        std::cout << "[ERROR] Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
}

int main(int argc, char **argv)
{
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //NOTE: mac only supports forward-compatible, core profile for v3.x & v4.x
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, argv[0], 0, 0);
    verifyWindow(window); 
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);

    // init OpenGL, vbo glsl after setting RC
    initGL();
    initGLSL();
    initVBO();

    // load bitmap font from array, call it after GL/GLSL is initialized
    bmFont.loadFont(fontCourier20, bitmapCourier20);

    // CAMARA
    // glfwMakeContextCurrent(window);
    // glfwSetCursorPosCallback(window, mouse_callback);
    // glfwSetScrollCallback(window, scroll_callback);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    string currentPath = getCurrentWorkingDirectory(); // el path termina en "/"
    string path_texture_moon = currentPath +  "sphere/bin/moon1024.bmp";
    string path_texture_sun = currentPath + "sphere/bin/8k_sun.bmp";
    texId = loadTexture(path_texture_moon.c_str(), false);
    texId2 = loadTexture(path_texture_sun.c_str(), false);
	
    projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    projection_sphere = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    // 1er modelo tiger
    Model model_tiger;
    model_tiger.loadShaders("TransformVertexShader.glsl", "TextureFragmentShader.glsl");
    model_tiger.loadTexture("tiger.dds");
    model_tiger.loadModel("tiger.obj");
    model_tiger.init();
    model_tiger.setProjection(projection);
    model_tiger.scale({0.2, 0.2, 0.2});
    model_tiger.translate({1.5, 0, 1});
    modelos.push_back(model_tiger);

    // 2do modelo llama
    Model model_llama;
    model_llama.loadShaders("TransformVertexShader.glsl", "TextureFragmentShader.glsl");
    model_llama.loadTexture("llama.dds");
    model_llama.loadModel("llama.obj");
    model_llama.init();
    model_llama.setProjection(projection);
    model_llama.scale({0.4, 0.4, 0.4});
    model_llama.translate({-1.5, 0, 2});
    modelos.push_back(model_llama);


    // 3er modelo panda
    Model model_panda;
    model_panda.loadShaders("TransformVertexShader.glsl", "TextureFragmentShader.glsl");
    model_panda.loadTexture("panda.dds");
    model_panda.loadModel("panda.obj");
    model_panda.init();
    model_panda.setProjection(projection);
    model_panda.rotate(-90, {0, 1, 0});
    model_panda.scale({0.4, 0.4, 0.4});
    model_panda.translate({-4, 0, -2});
    modelos.push_back(model_panda);

    // 4to modelo zorro
    Model model_zorro;
    model_zorro.loadShaders("TransformVertexShader.glsl", "TextureFragmentShader.glsl");
    model_zorro.loadTexture("zorro.dds");
    model_zorro.loadModel("zorro.obj");
    model_zorro.init();
    model_zorro.setProjection(projection);
    model_zorro.scale({0.2, 0.2, 0.2});
    model_zorro.translate({3, 0, -2});
    modelos.push_back(model_zorro);

    // 5to modelo casa
    Model model_casa;
    model_casa.loadShaders("TransformVertexShader.glsl", "TextureFragmentShader.glsl");
    model_casa.loadTexture("casa.dds");
    model_casa.loadModel("casa.obj");
    model_casa.init();
    model_casa.setProjection(projection);
    model_casa.scale({0.07, 0.07, 0.07});
    model_casa.translate({-4, 0, -8});
    modelos.push_back(model_casa);

    srand(static_cast<unsigned int>(time(nullptr)));

    for(int i = 0; i < 30; i++){
        Model model_cloud; 
        model_cloud.loadShaders("TransformVertexShader.glsl", "TextureFragmentShader.glsl");
        model_cloud.loadTexture("uvmap.jpg");
        model_cloud.loadModel("cloud.obj");
        model_cloud.init();
        model_cloud.setProjection(projection);
        float tam = random(1, 5);
        model_cloud.scale({tam, tam, tam});
        float tx = random(-15, 15), ty = random(6, 9), tz = random(-15, 15);
        model_cloud.translate({tx, ty, tz});
        model_cloud.setName("cloud");
        modelos.push_back(model_cloud);
    }
    
    Tree *tree;
    Transform transf;
    transform_arboles.push_back(transf);
    arboles.push_back(tree);
    init(0);

    // Crear objeto Grass
    Grass grass(currentPath + "grass/assets/shaders/", currentPath + "grass/assets/textures/grass_texture.png", currentPath + "grass/assets/textures/flowmap.png");
    // glPointSize(5.0f); // comment this line to reset points to normal size
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) {
            switch (key) {
                case GLFW_KEY_R:
                    crearArbol();
                    break;
                case GLFW_KEY_F:
                    eliminarUltimoArbol();
                    break;
                default:
                    break;
            }
        }
    });
    glfwSwapInterval(1); 
    while(!glfwWindowShouldClose(window))
    {
        double currTime = glfwGetTime();
        double frameTime = currTime - runTime;
        runTime = currTime;

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        static float prev_time = 0;

        float current_time = glfwGetTime();
        float time_delta = current_time - prev_time;
        prev_time = current_time;

        processInput(window);
        glfwMakeContextCurrent(window);
        bool vista = 1;
        display(time_delta, grass, frameTime, currTime);  
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // terminate program
    glfwTerminate();
    std::cout << "Terminate program" << std::endl;
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5 * deltaTime; 
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // adelante
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // abajo
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // izquierda
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // derecha
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) // arriba
        cameraPos += cameraUp * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) // abajo
        cameraPos -= cameraUp * cameraSpeed;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

std::string getCurrentWorkingDirectory()
{
    char cCurrentPath[FILENAME_MAX];
    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) { return std::string(); }
    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
    std::string currentPath = std::string(cCurrentPath);
    int found = currentPath.find("build");
    currentPath = currentPath.substr(0, found);
    unsigned int i = 0;
    while (i < currentPath.size()) {
        if (currentPath[i] == '\\') { currentPath[i] = '/'; }
        ++i;
    }
    return currentPath;
}

void init(int ind){
    GLuint cube_pid = load_shaders("cube_vshader.glsl", "cube_fshader.glsl");
    cube_base.init(cube_pid);
    cube_base.set_color(0.545f, 0.271f, 0.075f);
    cube_base_transf.scale(20.0f, 0.1f, 20.0f);
    //clip coord to tell shader not to draw anything over the water
    cube_base.set_clip_coord(0, 1, 0, -20);

    transform_arboles[ind].scale(1.5, 1.5, 1.5);
    // transform_arboles[ind].translate(2, 0, 0);
    time_measured = 0.0f;
    effect_select = 0;
    // FORMA DE MI ARBOL
    // tree_transf.translate(2.0f, 2.0f, 0.0f); // parece que la camara esta a la izquierda
    // tree_transf.scale(4.0f, 3.0f, 3.0f);

    trunk_pid = load_shaders("trunk_vshader.glsl", "trunk_fshader.glsl");
    leaves_pid = load_shaders("leaves_individual_vshader.glsl", "leaves_individual_fshader.glsl");

    arboles[ind] = new Tree;
    arboles[ind]->init(trunk_pid, leaves_pid); // cargar texturas
    arboles[ind]->load();

    //framebuffers
    framebuffer = new Framebuffer();
    GLuint tex_fb = framebuffer->init(SCR_WIDTH, SCR_HEIGHT, true);

    GLuint pid_quad_screen = load_shaders("quad_screen_vshader.glsl", "quad_screen_fshader.glsl");
    quad_screen.init(tex_fb, SCR_WIDTH, SCR_HEIGHT, pid_quad_screen);
}


void update_parameters(double currTime) {
    float baseIntensity = 1.0f; // Intensidad base
    float variationAmplitude = 0.5f; // Amplitud de la variación

    if (abs(rotationAngle - M_PI / 2.0) <= 0.01){ // Sol visible
        rotationAngle = -M_PI / 2.0;
        modoDia = !modoDia;
    } 

    // Colores del Sol
    glm::vec3 color_sol_amanecer = glm::vec3(0.8f, 0.6f, 0.4f); // Color cálido para el amanecer
    glm::vec3 color_sol_mediodia = glm::vec3(1.0f, 1.0f, 1.0f); // Color más fuerte para el mediodía

    // Colores de la Luna
    glm::vec3 color_luna_anochecer = glm::vec3(0.5f, 0.5f, 0.7f); // Color más intenso para el anochecer
    glm::vec3 color_luna_medianoche = glm::vec3(0.3f, 0.3f, 0.6f); // Color más fuerte para la medianoche
 


    float t = (rotationAngle + M_PI / 2.0) / M_PI;
    // Seleccionar el color del sol o la luna dependiendo de si es día o noche
    if (modoDia) {
        if (t < 0.65f) {
            sphere_col = mix(color_sol_amanecer, color_sol_mediodia, t);
            sun_col = mix(color_sol_amanecer, color_sol_mediodia, t);
            interpolatedColor = mix(memo1, background_color[2], t);
            memo2 = interpolatedColor;
            intensityGrass = baseIntensity + 0.8 * t;
        } else {
            sphere_col = mix(color_sol_mediodia, color_luna_anochecer, t);
            sun_col = mix(memo1, color_luna_anochecer, t);
            interpolatedColor = mix(memo2, background_color[1], t);
            intensityGrass = baseIntensity + 0.4 * t;
        }
    } 
    else {
        if (t < 0.65f) {
            sphere_col = mix(color_luna_anochecer, color_luna_medianoche, t);
            sun_col = mix(color_luna_anochecer, color_luna_medianoche, t);
            interpolatedColor = mix(background_color[1], {0.5, 0.5, 0.5}, t);
            intensityGrass = baseIntensity + 0.6* t;
        } else {
            sphere_col = mix(color_luna_medianoche, color_sol_amanecer, t);
            sun_col = mix(color_luna_medianoche, color_sol_amanecer, t);;
            interpolatedColor = mix(background_color[0], {0.7, 0.7, 0.7}, t);
            memo1 = interpolatedColor;
            intensityGrass = baseIntensity + 0.3* t;
        }
    }

    light_position[0] = posXSol;
    light_position[1] = posYSol;
    light_position[2] = posZSol;
}

void drawtree(int ind, float time_delta, float currTime) {
    GLfloat auxLightPosition[3] = {light_position[0], light_position[2], light_position[3]};
    cube_base.set_light_pos(auxLightPosition);
    GLfloat _camera_pos[3] = {cameraPos[0], cameraPos[1], cameraPos[2]};
    cube_base.set_camera_pos(_camera_pos);
    float _sun_col[3] = {sun_col[0], sun_col[1], sun_col[2]};
    cube_base.set_sun_col(_sun_col);
    cube_base.set_MVP_matrices(cube_base_transf.get_matrix(), glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp), projection);
    arboles[ind]->set_MVP_matrices(transform_arboles[ind].get_matrix(), glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp), projection);
    update_parameters(currTime);
    arboles[ind]->set_light_pos(light_position);
    arboles[ind]->set_camera_pos(_camera_pos);
    arboles[ind]->set_sun_col(_sun_col);
    arboles[ind]->set_shadow_buffer_texture_size(SCR_WIDTH, SCR_HEIGHT);
    arboles[ind]->set_window_dim(SCR_WIDTH, SCR_HEIGHT);
    arboles[ind]->move_leaves(time_delta);
    arboles[ind]->draw();
}



void display(float time_delta, Grass &pasto, double _frameTime, double currTime){
    framebuffer->bind();
    glClearColor(interpolatedColor[0], interpolatedColor[1], interpolatedColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    glViewport(0, SCR_HEIGHT / 8, SCR_WIDTH / 2, SCR_HEIGHT / 1.5);
    for(int i = 0; i < arboles.size(); i++){
        drawtree(i, time_delta, currTime);
    }
    pasto.draw();
    glm::mat4 aux4 = projection_sphere;
    projection_sphere = glm::perspective(glm::radians(10.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    drawSphere(_frameTime);
    projection_sphere = aux4;
    cube_base.draw();

    for(auto &model: modelos){
        model.setView(view);
        model.setCameraPos({cameraPos[0], cameraPos[1], cameraPos[2]});
        model.setLightPos({light_position[0], light_position[1], light_position[2]});
        model.setLightColor({sun_col[0], sun_col[1], sun_col[2]});
        if(model.name == "cloud"){
            model.rotate(0.7, {0, 1, 0});
        }
        model.draw();
    }
    framebuffer->unbind();
    quad_screen.draw(effect_select);
    

    framebuffer->bind();
    glm::vec3 aux =  cameraPos;
    glm::vec3 aux2 =  cameraFront;
    glm::mat4 aux3 = projection;
    cameraPos[0] = -27, cameraPos[1] = 2, cameraPos[2] = 0;
    cameraFront[0] = 1, cameraFront[1] = 0, cameraFront[2] = 0;
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    glViewport(SCR_WIDTH / 2, SCR_HEIGHT / 6, SCR_WIDTH / 2, SCR_HEIGHT / 2);
    
    for(int i = 0; i < arboles.size(); i++){
        drawtree(i, time_delta, currTime);
    }
    drawSphere(_frameTime);
    cube_base.draw();
    pasto.draw();

    for(auto &model: modelos){
        model.setView(view);
        model.setProjection(projection);
        model.setCameraPos({cameraPos[0], cameraPos[1], cameraPos[2]});
        model.setLightPos({light_position[0], light_position[1], light_position[2]});
        model.setLightColor({sun_col[0], sun_col[1], sun_col[2]});
        model.draw();
        model.setProjection(aux3);
    }
    framebuffer->unbind();
    quad_screen.draw(effect_select);
    cameraPos = aux;
    cameraFront = aux2;
    projection = aux3;
}
