cmake_minimum_required(VERSION 3.10)

# Project name
set(PROJECT_NAME grass)
project(${PROJECT_NAME})

# Set build type
set(CMAKE_BUILD_TYPE Release)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Find required packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

# Set include directories
set(INCLUDE_PATH 
    "/usr/include/GL" 
    "/usr/include/glm" 
    "${CMAKE_CURRENT_SOURCE_DIR}/../common"
    "/opt/homebrew/opt/glew/include"
    "/opt/homebrew/opt/glfw/include"
    "/opt/homebrew/opt/mesa/include"
    "/opt/homebrew/opt/devil/include"
    "/opt/homebrew/Cellar/glm/1.0.1/include"
    "/opt/homebrew/opt/freeglut"
    "${CMAKE_CURRENT_SOURCE_DIR}/sphere/src/glad/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/tree/_cube"
    "${CMAKE_CURRENT_SOURCE_DIR}/grass"
    "${CMAKE_CURRENT_SOURCE_DIR}/models"
)

# Set library directories
set(LIB_PATH 
    "/usr/lib"
    "/opt/homebrew/lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/sphere/src/glfw/lib/mac"
)

include_directories(${INCLUDE_PATH})
link_directories(${LIB_PATH})

# Add definitions
add_definitions(
    -DTW_STATIC
    -DTW_NO_LIB_PRAGMA
    -DTW_NO_DIRECT3D
    -DGLEW_STATIC
    -D_CRT_SECURE_NO_WARNINGS
)

# Gather source files for grass
file(GLOB GRASS_SOURCE_FILES 
    "*.cpp" 
    "*.h"
    "models/common/*hpp"
    "models/common/*cpp"
    "grass/common/*.cpp"
    "grass/common/*.hpp"
    "models/*.h"
)

# Gather source files for sphere
file(GLOB SPHERE_SOURCE_FILES
    "sphere/src/glad/src/glad.c"
    "sphere/src/Matrices.cpp"
    "sphere/src/Bmp.cpp"
    "sphere/src/Timer.cpp"
    "sphere/src/Tokenizer.cpp"
    "sphere/src/BitmapFontData.cpp"
    "sphere/src/Sphere.cpp"
    "sphere/src/main.cpp"
)

# Create executable for grass
add_executable(${PROJECT_NAME} ${GRASS_SOURCE_FILES} ${SPHERE_SOURCE_FILES})

# Copy all GLSL files to build folder
file(GLOB_RECURSE GLSL_FILES "*.glsl")
foreach(glsl_file ${GLSL_FILES})
    configure_file(${glsl_file} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endforeach(glsl_file)


# INICIO DEPENDENCIAS .OBJ
file(GLOB_RECURSE GLSL_FILES "models/*.glsl")
foreach(glsl_file ${GLSL_FILES})
    configure_file(${glsl_file} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endforeach(glsl_file)

file(GLOB_RECURSE GLSL_FILES "models/*.obj")
foreach(glsl_file ${GLSL_FILES})
    configure_file(${glsl_file} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endforeach(glsl_file)

file(GLOB_RECURSE GLSL_FILES "models/*.jpg")
foreach(glsl_file ${GLSL_FILES})
    configure_file(${glsl_file} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endforeach(glsl_file)

file(GLOB_RECURSE GLSL_FILES "models/*.dds")
foreach(glsl_file ${GLSL_FILES})
    configure_file(${glsl_file} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endforeach(glsl_file)
# FIN DEPENDENCIAS .OBJ

# Copy all BMP files to build folder
file(GLOB_RECURSE BMP_FILES "sphere/bin/*.bmp")
foreach(bmp_file ${BMP_FILES})
    configure_file(${bmp_file} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endforeach(bmp_file)

# Set libraries to link
set(ALL_LIBS
    ${OPENGL_LIBRARY}
    GLEW
    glfw
)

target_link_libraries(${PROJECT_NAME} ${ALL_LIBS})
