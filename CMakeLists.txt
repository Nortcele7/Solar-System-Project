cmake_minimum_required(VERSION 3.10)
project(SolarSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

add_executable(solar_system src/main.cpp)

target_link_libraries(solar_system 
    OpenGL::OpenGL
    glfw
    GLEW::GLEW
    glm::glm
)

target_include_directories(solar_system PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
