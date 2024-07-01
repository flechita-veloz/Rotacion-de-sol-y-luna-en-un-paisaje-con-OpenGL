///////////////////////////////////////////////////////////////////////////////
// main.cpp
// ========
// drawing a sphere using VBO & GLSL
// dependency: glad, glfw
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2017-11-02
// UPDATED: 2023-04-28
///////////////////////////////////////////////////////////////////////////////

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include "Matrices.h"
#include "Bmp.h"
#include "BitmapFontData.h"     // to draw bitmap font with GLFW
#include "fontCourier20.h"      // font:courier new, height:20px
#include "Timer.h"
#include "Sphere.h"
using namespace std;
// function prototypes
void initGL();
bool initGLSL();
void initVBO();
GLuint loadTexture(const char* fileName, bool wrap=true);

// Bui-Tuong Phong shading model with texture =================================
const char* vsSource = R"(
// GLSL version (OpenGL 3.3)
#version 330
// uniforms
uniform mat4 matrixModelView;
uniform mat4 matrixNormal;
uniform mat4 matrixModelViewProjection;
// vertex attribs (input)
layout(location=0) in vec3 vertexPosition;
layout(location=1) in vec3 vertexNormal;
layout(location=2) in vec2 vertexTexCoord;
// varyings (output)
out vec3 esVertex;
out vec3 esNormal;
out vec2 texCoord0;
void main()
{
    esVertex = vec3(matrixModelView * vec4(vertexPosition, 1.0));
    esNormal = vec3(matrixNormal * vec4(vertexNormal, 1.0));
    texCoord0 = vertexTexCoord;
    gl_Position = matrixModelViewProjection * vec4(vertexPosition, 1.0);
}
)";

const char* fsSource = R"(
// GLSL version (OpenGL 3.3)
#version 330
// uniforms
uniform vec4 lightPosition;             // should be in the eye space
uniform vec4 lightAmbient;              // light ambient color
uniform vec4 lightDiffuse;              // light diffuse color
uniform vec4 lightSpecular;             // light specular color
uniform vec4 materialAmbient;           // material ambient color
uniform vec4 materialDiffuse;           // material diffuse color
uniform vec4 materialSpecular;          // material specular color
uniform float materialShininess;        // material specular shininess
uniform sampler2D map0;                 // texture map #1
uniform bool textureUsed;               // flag for texture

uniform vec3 sun_col;                 // color of light 
uniform vec3 sphere_col;                 // color of the sphere 


// varyings (input)
in vec3 esVertex;
in vec3 esNormal;
in vec2 texCoord0;
// output
out vec4 fragColor;
void main()
{
    vec3 normal = normalize(esNormal);
    vec3 light;
    if(lightPosition.w == 0.0)
    {
        light = normalize(lightPosition.xyz);
    }
    else
    {
        light = normalize(lightPosition.xyz - esVertex);
    }
    vec3 view = normalize(-esVertex);
    vec3 reflectVec = reflect(-light, normal);  // 2 * N * (N dot L) - L

    vec3 color = lightAmbient.rgb * materialAmbient.rgb;        // begin with ambient
    float dotNL = max(dot(normal, light), 0.0);
    color += lightDiffuse.rgb * materialDiffuse.rgb * dotNL;    // add diffuse
    if(textureUsed)
    color *= texture(map0, texCoord0).rgb;                  // modulate texture map
    float dotVR = max(dot(view, reflectVec), 0.0);
    color += pow(dotVR, materialShininess) * lightSpecular.rgb * materialSpecular.rgb; // add specular

    color *= sphere_col;
    fragColor = vec4(color, materialDiffuse.a);                 // set frag color
}
)";


// Parametros de Animacion
float rotationAngle = -M_PI / 2.0; // Ángulo de rotación alrededor del eje X
float ellipseRadiusY = 10.0f; // Radio en el eje Y de la elipse
float ellipseRadiusZ = 15.0f; // Radio en el eje Z de la elipse
float posXSol, posYSol, posZSol;
float posXLuna, posYLuna, posZLuna;
bool modoDia = 1;
float rotationSpeed = 0.2f; // Velocidad de rotación
glm::vec3 sun_col;
glm::vec3 sphere_col;


double runTime;
int drawMode;
GLuint vaoId1, vaoId2;      // IDs of VAO for vertex array states
GLuint vboId1, vboId2;      // IDs of VBO for vertex arrays
GLuint iboId1, iboId2;      // IDs of VBO for index array
GLuint texId;
GLuint texId2;
BitmapFontData bmFont;
Matrix4 matrixModelView;
Matrix4 matrixProjection;
// GLSL
GLuint progId = 0;                  // ID of GLSL program
GLint uniformMatrixModelView;
GLint uniformMatrixModelViewProjection;
GLint uniformMatrixNormal;
GLint uniformLightPosition;
GLint uniformLightAmbient;
GLint uniformLightDiffuse;
GLint uniformLightSpecular;
GLint uniformMaterialAmbient;
GLint uniformMaterialDiffuse;
GLint uniformMaterialSpecular;
GLint uniformMaterialShininess;
GLint uniformMap0;
GLint uniformTextureUsed;
GLint attribVertexPosition;     // 0
GLint attribVertexNormal;       // 1
GLint attribVertexTexCoord;     // 2
GLint uniformSunCol; // color del luz
GLint uniformSphereCol; // color de esfera



// sphere: min sector = 3, min stack = 2
Sphere sphere1(1.0f, 36, 18, true, 2); // radius, sectors, stacks, non-smooth (flat) shading, Y-up
Sphere sphere2(1.0f, 36, 18, true, 2);  // radius, sectors, stacks, smooth(default), Y-up

glm::mat4 projection_sphere;
glm::mat4 view, view2;

///////////////////////////////////////////////////////////////////////////////
// initialize OpenGL
// disable unused features
///////////////////////////////////////////////////////////////////////////////
void initGL()
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

    // enable /disable features
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glClearColor(0, 0, 0, 0);                   // background color
    glClearStencil(0);                          // clear stencil buffer
    glClearDepth(1.0f);                         // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);
}

bool initGLSL()
{
    const int MAX_LENGTH = 2048;
    char log[MAX_LENGTH];
    int logLength = 0;

    // create shader and program
    GLuint vsId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fsId = glCreateShader(GL_FRAGMENT_SHADER);
    progId = glCreateProgram();

    // load shader sources
    glShaderSource(vsId, 1, &vsSource, NULL);
    glShaderSource(fsId, 1, &fsSource, NULL);

    // compile shader sources
    glCompileShader(vsId);
    glCompileShader(fsId);

    //@@ debug
    int vsStatus, fsStatus;
    glGetShaderiv(vsId, GL_COMPILE_STATUS, &vsStatus);
    if(vsStatus == GL_FALSE)
    {
        glGetShaderiv(vsId, GL_INFO_LOG_LENGTH, &logLength);
        glGetShaderInfoLog(vsId, MAX_LENGTH, &logLength, log);
        std::cout << "===== Vertex Shader Log =====\n" << log << std::endl;
    }
    glGetShaderiv(fsId, GL_COMPILE_STATUS, &fsStatus);
    if(fsStatus == GL_FALSE)
    {
        glGetShaderiv(fsId, GL_INFO_LOG_LENGTH, &logLength);
        glGetShaderInfoLog(fsId, MAX_LENGTH, &logLength, log);
        std::cout << "===== Fragment Shader Log =====\n" << log << std::endl;
    }

    // attach shaders to the program
    glAttachShader(progId, vsId);
    glAttachShader(progId, fsId);

    // link program
    glLinkProgram(progId);

    // get uniform/attrib locations
    glUseProgram(progId);
    uniformMatrixModelView           = glGetUniformLocation(progId, "matrixModelView");
    uniformMatrixModelViewProjection = glGetUniformLocation(progId, "matrixModelViewProjection");
    uniformMatrixNormal              = glGetUniformLocation(progId, "matrixNormal");
    uniformLightPosition             = glGetUniformLocation(progId, "lightPosition");
    uniformLightAmbient              = glGetUniformLocation(progId, "lightAmbient");
    uniformLightDiffuse              = glGetUniformLocation(progId, "lightDiffuse");
    uniformLightSpecular             = glGetUniformLocation(progId, "lightSpecular");
    uniformMaterialAmbient           = glGetUniformLocation(progId, "materialAmbient");
    uniformMaterialDiffuse           = glGetUniformLocation(progId, "materialDiffuse");
    uniformMaterialSpecular          = glGetUniformLocation(progId, "materialSpecular");
    uniformMaterialShininess         = glGetUniformLocation(progId, "materialShininess");
    uniformMap0                      = glGetUniformLocation(progId, "map0");
    uniformTextureUsed               = glGetUniformLocation(progId, "textureUsed");
    uniformSunCol                 = glGetUniformLocation(progId, "sun_col");
    attribVertexPosition = glGetAttribLocation(progId, "vertexPosition");
    attribVertexNormal   = glGetAttribLocation(progId, "vertexNormal");
    attribVertexTexCoord = glGetAttribLocation(progId, "vertexTexCoord");
    attribVertexTexCoord = glGetAttribLocation(progId, "vertexTexCoord");
    uniformSphereCol = glGetUniformLocation(progId, "sphere_col");

    float lightPosition[] = {-1, 0, 1, 1}; // 1 luz posicional, 0 luz direccional
    float lightAmbient[]  = {1.0f, 1.0f, 1.0f, 1};
    float lightDiffuse[]  = {1.0f, 1.0f, 1.0f, 1};
    float lightSpecular[] = {1.0f, 1.0f, 1.0f, 1};
    float materialAmbient[]  = {0.5f, 0.5f, 0.5f, 1};
    float materialDiffuse[]  = {0.7f, 0.7f, 0.7f, 1};
    float materialSpecular[] = {0.4f, 0.4f, 0.4f, 1};
    float materialShininess  = 16;
    
    glUniform4fv(uniformLightPosition, 1, lightPosition);
    glUniform4fv(uniformLightAmbient, 1, lightAmbient);
    glUniform4fv(uniformLightDiffuse, 1, lightDiffuse);
    glUniform4fv(uniformLightSpecular, 1, lightSpecular);
    glUniform4fv(uniformMaterialAmbient, 1, materialAmbient);
    glUniform4fv(uniformMaterialDiffuse, 1, materialDiffuse);
    glUniform4fv(uniformMaterialSpecular, 1, materialSpecular);
    glUniform1f(uniformMaterialShininess, materialShininess);
    glUniform1i(uniformMap0, 0);
    glUniform1i(uniformTextureUsed, 1);

    // unbind GLSL
    glUseProgram(0);
    glDeleteShader(vsId);
    glDeleteShader(fsId);

    // check GLSL status
    int linkStatus;
    glGetProgramiv(progId, GL_LINK_STATUS, &linkStatus);
    if(linkStatus == GL_FALSE)
    {
        glGetProgramiv(progId, GL_INFO_LOG_LENGTH, &logLength);
        glGetProgramInfoLog(progId, MAX_LENGTH, &logLength, log);
        std::cout << "===== GLSL Program Log =====\n" << log << std::endl;
        return false;
    }
    else
    {
        return true;
    }
}



///////////////////////////////////////////////////////////////////////////////
// copy vertex data to VBO and VA state to VAO
///////////////////////////////////////////////////////////////////////////////
void initVBO()
{
    // sphere1
    // create vertex array object to store all vertex array states only once
    if(!vaoId1)
        glGenVertexArrays(1, &vaoId1);
    glBindVertexArray(vaoId1);

    // create vertex buffer objects
    if(!vboId1)
        glGenBuffers(1, &vboId1);

    glBindBuffer(GL_ARRAY_BUFFER, vboId1);
    glBufferData(GL_ARRAY_BUFFER, sphere1.getInterleavedVertexSize(), sphere1.getInterleavedVertices(), GL_STATIC_DRAW);

    if(!iboId1)
        glGenBuffers(1, &iboId1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere1.getIndexSize(), sphere1.getIndices(), GL_STATIC_DRAW);

    // enable vertex array attributes for bound VAO
    glEnableVertexAttribArray(attribVertexPosition);
    glEnableVertexAttribArray(attribVertexNormal);
    glEnableVertexAttribArray(attribVertexTexCoord);

    // store vertex array pointers to bound VAO
    int stride = sphere1.getInterleavedStride();
    glVertexAttribPointer(attribVertexPosition, 3, GL_FLOAT, false, stride, 0);
    glVertexAttribPointer(attribVertexNormal, 3, GL_FLOAT, false, stride, (void*)(3 * sizeof(float)));
    glVertexAttribPointer(attribVertexTexCoord, 2, GL_FLOAT, false, stride, (void*)(6 * sizeof(float)));

    // sphere2
    if(!vaoId2)
        glGenVertexArrays(1, &vaoId2);
    glBindVertexArray(vaoId2);

    if(!vboId2)
        glGenBuffers(1, &vboId2);

    glBindBuffer(GL_ARRAY_BUFFER, vboId2);
    glBufferData(GL_ARRAY_BUFFER, sphere2.getInterleavedVertexSize(), sphere2.getInterleavedVertices(), GL_STATIC_DRAW);

    if(!iboId2)
        glGenBuffers(1, &iboId2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere2.getIndexSize(), sphere2.getIndices(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(attribVertexPosition);
    glEnableVertexAttribArray(attribVertexNormal);
    glEnableVertexAttribArray(attribVertexTexCoord);

    stride = sphere2.getInterleavedStride();
    glVertexAttribPointer(attribVertexPosition, 3, GL_FLOAT, false, stride, 0);
    glVertexAttribPointer(attribVertexNormal, 3, GL_FLOAT, false, stride, (void*)(3 * sizeof(float)));
    glVertexAttribPointer(attribVertexTexCoord, 2, GL_FLOAT, false, stride, (void*)(6 * sizeof(float)));

    // unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


///////////////////////////////////////////////////////////////////////////////
// load raw image as a texture
///////////////////////////////////////////////////////////////////////////////
GLuint loadTexture(const char* fileName, bool wrap)
{
    Image::Bmp bmp;
    if(!bmp.read(fileName))
        return 0;     // exit if failed load image

    // get bmp info
    int width = bmp.getWidth();
    int height = bmp.getHeight();
    const unsigned char* data = bmp.getDataRGB();
    GLenum type = GL_UNSIGNED_BYTE;    // only allow BMP with 8-bit per channel

    // We assume the image is 8-bit, 24-bit or 32-bit BMP
    GLenum format;
    int bpp = bmp.getBitCount();
    if(bpp == 8)
        format = GL_LUMINANCE;
    else if(bpp == 24)
        format = GL_RGB;
    else if(bpp == 32)
        format = GL_RGBA;
    else
        return 0;               // NOT supported, exit

    // gen texture ID
    GLuint texture;
    glGenTextures(1, &texture);

    // set active texture and configure it
    glBindTexture(GL_TEXTURE_2D, texture);

    // select modulate to mix texture with color for shading
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // copy texture data
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}


float cnt = 0;
void drawSphere(double frameTime)
{
    rotationAngle += rotationSpeed * frameTime; // Ajusta el ángulo según el tiempo del frame
    if (rotationAngle > 2 * M_PI){
        rotationAngle -= 2 * M_PI;
    }
    Matrix4 matrixModelCommon;
    cnt += 0.7;
    matrixModelCommon.rotateY(cnt);
    matrixModelCommon.rotateX(cnt);
    Matrix4 matrixModelSol(matrixModelCommon);

    posXSol = 0;
    posYSol = ellipseRadiusY * cos(rotationAngle);
    posZSol = -ellipseRadiusZ * sin(rotationAngle);

    glUseProgram(progId);
    glActiveTexture(GL_TEXTURE0);
    Matrix4 matrixView;
    // Matrix4 matrixModelSol;
    for (int i = 0; i < 4; ++i) {
        const float row[4] = {projection_sphere[i][0], projection_sphere[i][1], projection_sphere[i][2], projection_sphere[i][3]};
        const float row2[4] = {view[i][0], view[i][1], view[i][2], view[i][3]};
        matrixView.setColumn(i, row2);
        matrixProjection.setColumn(i, row);
    }
    matrixModelSol.translate(posXSol, posYSol, posZSol);
    // matrixModelSol.scale(2, 2, 2);
    matrixModelView = matrixView * matrixModelSol;
    Matrix4 matrixModelViewProjection = matrixProjection * matrixModelView;
    Matrix4 matrixNormal = matrixModelView;
    matrixNormal.setColumn(3, Vector4(0,0,0,1));

    glUniformMatrix4fv(uniformMatrixModelView, 1, GL_FALSE, matrixModelView.get());
    glUniformMatrix4fv(uniformMatrixModelViewProjection, 1, GL_FALSE, matrixModelViewProjection.get());
    glUniformMatrix4fv(uniformMatrixNormal, 1, GL_FALSE, matrixNormal.get());
    GLfloat aux[3] = {sun_col[0], sun_col[1], sun_col[2]};
    glUniform3fv(uniformSunCol, 1, aux);
    GLfloat aux2[3] = {sphere_col[0], sphere_col[1], sphere_col[2]};
    glUniform3fv(uniformSphereCol, 1, aux2);

    if(modoDia){
        glBindTexture(GL_TEXTURE_2D, texId2);
        glUniform1i(uniformTextureUsed, 1);
    }
    else{
        glBindTexture(GL_TEXTURE_2D, texId);
        glUniform1i(uniformTextureUsed, 1);
    }

    // Draw animated sphere (sol)
    glBindVertexArray(vaoId2);
    glDrawElements(GL_TRIANGLES,            // primitive type
                sphere2.getIndexCount(), // # of indices
                GL_UNSIGNED_INT,         // data type
                (void*)0);               // ptr to indices
    
    // Unbind
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}


