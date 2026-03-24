#include <GL/glew.h>  // Loads OPENGL extensions
#include <GLFW/glfw3.h> // window create garcha ra input handle garcha
#include <glm/glm.hpp> // Provides math i.e vectors and matrices
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "../include/Sphere.h"

namespace {
constexpr float PI = 3.14159265359f;   
constexpr float TWO_PI = 6.28318530718f;

// Vertex and Fragment shader 
// 4 ota shader pins cha each living as a raw string in C++

const char* kBodyVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * worldPos;
}
)";

const char* kBodyFragmentShader = R"(
#version 330 core
in vec3 FragPos;
in vec3 Normal;

uniform vec3 baseColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float ambientStrength;
uniform float emissiveStrength;
uniform float earthMask;
uniform float earthPhase;

out vec4 FragColor;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    vec3 albedo = baseColor;
    if (earthMask > 0.5) {
        float longitude = atan(norm.z, norm.x);
        float latitude = asin(clamp(norm.y, -1.0, 1.0));
        float shape = 0.55 * sin(longitude * 5.0 + earthPhase)
                    + 0.35 * sin(latitude * 9.0 - earthPhase * 0.6)
                    + 0.25 * cos((longitude + latitude) * 7.0 + earthPhase * 0.4);
        float land = smoothstep(0.10, 0.42, shape);
        vec3 oceanColor = vec3(0.10, 0.40, 0.82);
        vec3 landColor = vec3(0.18, 0.62, 0.24);
        albedo = mix(oceanColor, landColor, land);
    }

    float diff = max(dot(norm, lightDir), 0.0);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 48.0);

    vec3 ambient = ambientStrength * albedo;
    vec3 diffuse = diff * albedo;
    vec3 specular = 0.35 * spec * vec3(1.0);
    vec3 emissive = emissiveStrength * albedo;

    vec3 color = ambient + diffuse + specular + emissive;
    FragColor = vec4(color, 1.0);
}
)";

// Line Shaders are basically used to draw elliptical orbits of planets and moons

const char* kLineVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* kLineFragmentShader = R"(
#version 330 core
uniform vec3 lineColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(lineColor, 1.0);
}
)";

// Point shader chai asteroid belt, kuiper belt, starfield ma use huncha to render points as small glowing dots in space

const char* kPointVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;
uniform float pointSize;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
    gl_PointSize = pointSize;
}
)";

const char* kPointFragmentShader = R"(
#version 330 core
uniform vec3 pointColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(pointColor, 1.0);
}
)";

// Body Shader

// Body Vertex Shader : Transforms each vertex from object space to world space to clip space
// Vertex shader uses model x view x projection pipeline

// It passes the world position and surface  normal to the fragment shader

/// Body Fragment Shader : Calculates the color of each pixel based on lighting and material properties
// Jati pani planets ma hune lightning haru cha tyo sab Body Fragment Shader ma calculate huncha

// Has a special case for Earth to create a procedural land/ocean pattern based on the vertex normal and an animation phase


// This is a bit map based version of letters in which each pixel is turned on off based on the character. Yesle chai text rendering ma help garcha
std::array<uint8_t, 7> GlyphPattern(char c) {
    switch (c) {
        case 'A': return {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
        case 'B': return {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E};
        case 'C': return {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E};
        case 'D': return {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E};
        case 'E': return {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
        case 'F': return {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};
        case 'G': return {0x0E, 0x11, 0x10, 0x10, 0x13, 0x11, 0x0F};
        case 'H': return {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
        case 'I': return {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F};
        case 'J': return {0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0E};
        case 'K': return {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11};
        case 'L': return {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
        case 'M': return {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11};
        case 'N': return {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11};
        case 'O': return {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
        case 'P': return {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
        case 'Q': return {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D};
        case 'R': return {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11};
        case 'S': return {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
        case 'T': return {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
        case 'U': return {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
        case 'V': return {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04};
        case 'W': return {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11};
        case 'X': return {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11};
        case 'Y': return {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04};
        case 'Z': return {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F};
        case '0': return {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E};
        case '1': return {0x04, 0x0C, 0x14, 0x04, 0x04, 0x04, 0x1F};
        case '2': return {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F};
        case '3': return {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E};
        case '4': return {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02};
        case '5': return {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E};
        case '6': return {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E};
        case '7': return {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08};
        case '8': return {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E};
        case '9': return {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C};
        case '.': return {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C};
        case ',': return {0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x08};
        case '-': return {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00};
        case ':': return {0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00};
        case '\'': return {0x0C, 0x0C, 0x08, 0x00, 0x00, 0x00, 0x00};
        case '%': return {0x19, 0x19, 0x02, 0x04, 0x08, 0x13, 0x13};
        case '/': return {0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10};
        case '(': return {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02};
        case ')': return {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08};
        default: return {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    }
}

std::string ToUpperAscii(std::string text) {
    for (char& ch : text) {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    return text;
}

std::vector<std::string> WrapText(const std::string& text, size_t maxChars) {
    std::istringstream iss(text);
    std::string word;
    std::vector<std::string> lines;
    std::string current;

    while (iss >> word) {
        if (current.empty()) {
            current = word;
            continue;
        }

        if (current.size() + 1 + word.size() <= maxChars) {
            current += " " + word;
        } else {
            lines.push_back(current);
            current = word;
        }
    }

    if (!current.empty()) {
        lines.push_back(current);
    }
    return lines;
}

GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compilation failed: " << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint CreateProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!vertex || !fragment) {
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "Program linking failed: " << log << std::endl;
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return program;
}

struct RingVertex {
    glm::vec3 position;
    glm::vec3 normal;
};

// Yo chai saturn ko rings create garna use huncha
// XZ plane ma donut shape ring banaucha
// Inner or Outer vertices around a circle banaucha
// Tyo vertices lai triangles ma convert garcha with an index buffer
// Then OPENGL renders it

class RingMesh {
public:
    RingMesh(float innerRadius, float outerRadius, int segments) {
        Generate(innerRadius, outerRadius, segments);
        Setup();
    }

    // Destructor for OPENGL objects

    ~RingMesh() {
        glDeleteBuffers(1, &ebo_);
        glDeleteBuffers(1, &vbo_);
        glDeleteVertexArrays(1, &vao_);
    }

    void Draw() const {
        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices_.size()), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

private:
    void Generate(float innerRadius, float outerRadius, int segments) {
        vertices_.clear();
        indices_.clear();

        for (int i = 0; i <= segments; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(segments);
            float angle = t * TWO_PI;
            float c = std::cos(angle);
            float s = std::sin(angle);
            glm::vec3 normal(0.0f, 1.0f, 0.0f);

            vertices_.push_back({glm::vec3(innerRadius * c, 0.0f, innerRadius * s), normal});
            vertices_.push_back({glm::vec3(outerRadius * c, 0.0f, outerRadius * s), normal});
        }

        for (int i = 0; i < segments; ++i) {
            unsigned int base = static_cast<unsigned int>(i * 2);
            indices_.push_back(base);
            indices_.push_back(base + 1);
            indices_.push_back(base + 2);

            indices_.push_back(base + 1);
            indices_.push_back(base + 3);
            indices_.push_back(base + 2);
        }
    }

    void Setup() {
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(vertices_.size() * sizeof(RingVertex)),
                     vertices_.data(),
                     GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(indices_.size() * sizeof(unsigned int)),
                     indices_.data(),
                     GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RingVertex), (void*)offsetof(RingVertex, position));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(RingVertex), (void*)offsetof(RingVertex, normal));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    std::vector<RingVertex> vertices_;
    std::vector<unsigned int> indices_;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
};

// Contains data for each celestial body. Contains orbital state and animation too.
// Yesle sabai planetes ko data store garera rakheko huncha

struct CelestialBody {
    std::string name;
    std::string description;
    float visualRadius;
    float orbitRadius;
    float orbitalPeriodDays;
    float rotationPeriodHours;
    float axialTiltDeg;  // Uranus ko lagi rotation axis tilt garcha its nearly 98 degrees
    float orbitInclinationDeg;
    float eccentricity;
    glm::vec3 color;
    float emissive;
    int parentIndex;
    bool drawOrbit;
    bool hasRing;
    float ringInnerScale;
    float ringOuterScale;
    float orbitAngle;
    float spinAngle;
    Sphere* sphere;
};

class SolarSystem {
public:
    bool Initialize(int width, int height) {
        width_ = width;
        height_ = height;


        // OpenGL context create garna GLFW use garcha. Yesle chai window create garcha ra input handle garcha
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window_ = glfwCreateWindow(width_, height_, "Interactive Solar Sytem Simulator | SSS", nullptr, nullptr);
        if (!window_) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window_);
        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, FramebufferSizeCallback);
        glfwSwapInterval(1);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            return false;
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glClearColor(0.01f, 0.01f, 0.03f, 1.0f);

        bodyProgram_ = CreateProgram(kBodyVertexShader, kBodyFragmentShader);
        lineProgram_ = CreateProgram(kLineVertexShader, kLineFragmentShader);
        pointProgram_ = CreateProgram(kPointVertexShader, kPointFragmentShader);

        if (!bodyProgram_ || !lineProgram_ || !pointProgram_) {
            return false;
        }

        BuildBodies();
        BuildOrbitMesh();
        BuildStarfield(2400, 260.0f);
        BuildBelts(2600, 4200);
        saturnRing_ = new RingMesh(1.45f, 2.55f, 128);

        worldPositions_.assign(bodies_.size(), glm::vec3(0.0f));
        ComputeWorldPositions();

        desiredCameraDistance_ = cameraDistance_;
        desiredCameraTarget_ = cameraTarget_;

        return true;
    }

    void Run() {
        double lastTime = glfwGetTime();

        PrintControls();

        while (!glfwWindowShouldClose(window_)) {
            double now = glfwGetTime();
            float dt = static_cast<float>(now - lastTime);
            lastTime = now;

            HandleInput(dt);
            Update(dt);
            Render();

            glfwPollEvents();
        }
    }

    void Shutdown() {
        for (CelestialBody& body : bodies_) {
            delete body.sphere;
            body.sphere = nullptr;
        }

        delete saturnRing_;
        saturnRing_ = nullptr;

        glDeleteBuffers(1, &orbitVbo_);
        glDeleteVertexArrays(1, &orbitVao_);

        glDeleteBuffers(1, &starVbo_);
        glDeleteVertexArrays(1, &starVao_);

        glDeleteBuffers(1, &asteroidBeltVbo_);
        glDeleteVertexArrays(1, &asteroidBeltVao_);

        glDeleteBuffers(1, &kuiperBeltVbo_);
        glDeleteVertexArrays(1, &kuiperBeltVao_);

        glDeleteProgram(bodyProgram_);
        glDeleteProgram(lineProgram_);
        glDeleteProgram(pointProgram_);

        if (window_) {
            glfwDestroyWindow(window_);
        }
        glfwTerminate();
    }

private:
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
        auto* system = static_cast<SolarSystem*>(glfwGetWindowUserPointer(window));
        if (!system) {
            return;
        }
        system->width_ = width;
        system->height_ = height;
        glViewport(0, 0, width, height);
    }

    void PrintControls() const {
        std::cout << "Controls:\n";
        std::cout << "Arrow keys: Orbit camera\n";
        std::cout << "W/S: Zoom in/out\n";
        std::cout << "A/D: Decrease/increase simulation speed\n";
        std::cout << "B: Toggle asteroid and Kuiper belts\n";
        std::cout << "Space: Pause/resume simulation\n";
        std::cout << "Left mouse: Select body and zoom to it\n";
        std::cout << "Right mouse or R: Reset focus\n";
        std::cout << "Esc: Exit\n";
    }

    void BuildBodies() {
        bodies_.clear();

        auto addBody = [this](const std::string& name,
                              const std::string& description,
                              float radius,
                              float orbitRadius,
                              float orbitalPeriodDays,
                              float rotationHours,
                              float axialTilt,
                              float orbitInclination,
                              float eccentricity,
                              const glm::vec3& color,
                              float emissive,
                              int parentIndex,
                              bool drawOrbit,
                              bool hasRing,
                              float ringInner,
                              float ringOuter,
                              int tessellation) {
            bodies_.push_back({name,
                               description,
                               radius,
                               orbitRadius,
                               orbitalPeriodDays,
                               rotationHours,
                               axialTilt,
                               orbitInclination,
                               eccentricity,
                               color,
                               emissive,
                               parentIndex,
                               drawOrbit,
                               hasRing,
                               ringInner,
                               ringOuter,
                               0.0f,
                               0.0f,
                               new Sphere(1.0f, tessellation, tessellation / 2)});
        };

        addBody("Sun", "The Sun is a G-type star containing 99.86% of the Solar System mass.",
                2.8f, 0.0f, 1.0f, 609.12f, 7.25f, 0.0f, 0.0f,
                glm::vec3(1.0f, 0.86f, 0.42f), 1.75f, -1, false, false, 0.0f, 0.0f, 72);

        addBody("Mercury", "Smallest planet and closest to the Sun, with extreme day-night temperature swings.",
                0.16f, 6.2f, 88.0f, 1407.6f, 0.03f, 7.0f, 0.2056f,
                glm::vec3(0.67f, 0.66f, 0.63f), 0.0f, 0, true, false, 0.0f, 0.0f, 32);

        addBody("Venus", "A dense CO2 atmosphere and runaway greenhouse effect make Venus the hottest planet.",
                0.40f, 8.8f, 224.7f, -5832.5f, 177.4f, 3.39f, 0.0068f,
                glm::vec3(0.93f, 0.79f, 0.55f), 0.0f, 0, true, false, 0.0f, 0.0f, 40);

        addBody("Earth", "Our ocean-rich home world with active plate tectonics and a nitrogen-oxygen atmosphere.",
                0.42f, 11.9f, 365.25f, 24.0f, 23.44f, 0.0f, 0.0167f,
                glm::vec3(0.26f, 0.53f, 0.93f), 0.0f, 0, true, false, 0.0f, 0.0f, 48);

        addBody("Moon", "Earth's only natural satellite, tidally locked and driving ocean tides.",
                0.11f, 1.3f, 27.32f, 655.7f, 6.68f, 5.15f, 0.055f,
                glm::vec3(0.75f, 0.74f, 0.72f), 0.0f, 3, false, false, 0.0f, 0.0f, 24);

        addBody("Mars", "The red planet with polar caps, giant volcanoes, and evidence of ancient water flows.",
                0.22f, 15.8f, 687.0f, 24.6f, 25.2f, 1.85f, 0.0934f,
                glm::vec3(0.86f, 0.41f, 0.24f), 0.0f, 0, true, false, 0.0f, 0.0f, 36);

        addBody("Jupiter", "Largest planet and a gas giant with a powerful magnetic field and many moons.",
                1.12f, 22.2f, 4331.0f, 9.9f, 3.13f, 1.30f, 0.0489f,
                glm::vec3(0.82f, 0.70f, 0.56f), 0.0f, 0, true, false, 0.0f, 0.0f, 56);

        addBody("Saturn", "Gas giant known for its bright ring system made of ice and rocky particles.",
                0.96f, 30.3f, 10747.0f, 10.7f, 26.7f, 2.49f, 0.0565f,
                glm::vec3(0.89f, 0.79f, 0.58f), 0.0f, 0, true, true, 1.45f, 2.55f, 56);

        addBody("Uranus", "Ice giant rotating on its side, giving it extreme seasons during its long orbit.",
                0.70f, 37.6f, 30589.0f, -17.2f, 97.8f, 0.77f, 0.0472f,
                glm::vec3(0.59f, 0.84f, 0.88f), 0.0f, 0, true, false, 0.0f, 0.0f, 48);

        addBody("Neptune", "Distant ice giant with supersonic winds and a deep blue methane-rich atmosphere.",
                0.68f, 43.8f, 59800.0f, 16.1f, 28.3f, 1.77f, 0.0086f,
                glm::vec3(0.34f, 0.48f, 0.90f), 0.0f, 0, true, false, 0.0f, 0.0f, 48);
    }

    void BuildOrbitMesh() {
        std::vector<glm::vec3> orbit;
        orbit.reserve(kOrbitSegments);
        for (int i = 0; i < kOrbitSegments; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(kOrbitSegments);
            float a = t * TWO_PI;
            orbit.push_back(glm::vec3(std::cos(a), 0.0f, std::sin(a)));
        }

        glGenVertexArrays(1, &orbitVao_);
        glGenBuffers(1, &orbitVbo_);
        glBindVertexArray(orbitVao_);
        glBindBuffer(GL_ARRAY_BUFFER, orbitVbo_);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(orbit.size() * sizeof(glm::vec3)),
                     orbit.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    void BuildStarfield(int starCount, float radius) {
        std::vector<glm::vec3> stars;
        stars.reserve(static_cast<size_t>(starCount));

        std::mt19937 rng(42);
        std::uniform_real_distribution<float> u01(0.0f, 1.0f);

        for (int i = 0; i < starCount; ++i) {
            float theta = TWO_PI * u01(rng);
            float phi = std::acos(1.0f - 2.0f * u01(rng));
            float r = radius + 20.0f * u01(rng);

            float x = r * std::sin(phi) * std::cos(theta);
            float y = r * std::cos(phi);
            float z = r * std::sin(phi) * std::sin(theta);
            stars.push_back(glm::vec3(x, y, z));
        }

        starCount_ = static_cast<GLsizei>(stars.size());

        glGenVertexArrays(1, &starVao_);
        glGenBuffers(1, &starVbo_);
        glBindVertexArray(starVao_);
        glBindBuffer(GL_ARRAY_BUFFER, starVbo_);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(stars.size() * sizeof(glm::vec3)),
                     stars.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    void BuildBelts(int asteroidCount, int kuiperCount) {
        std::mt19937 rng(84);
        std::uniform_real_distribution<float> u01(0.0f, 1.0f);

        std::vector<glm::vec3> asteroid;
        asteroid.reserve(static_cast<size_t>(asteroidCount));

        for (int i = 0; i < asteroidCount; ++i) {
            float radius = 18.2f + 3.6f * u01(rng);
            float angle = TWO_PI * u01(rng);
            float thickness = (u01(rng) - 0.5f) * 0.8f;
            float jitter = 0.35f * (u01(rng) - 0.5f);
            asteroid.push_back(glm::vec3((radius + jitter) * std::cos(angle), thickness, (radius + jitter) * std::sin(angle)));
        }

        asteroidBeltCount_ = static_cast<GLsizei>(asteroid.size());
        glGenVertexArrays(1, &asteroidBeltVao_);
        glGenBuffers(1, &asteroidBeltVbo_);
        glBindVertexArray(asteroidBeltVao_);
        glBindBuffer(GL_ARRAY_BUFFER, asteroidBeltVbo_);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(asteroid.size() * sizeof(glm::vec3)),
                     asteroid.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        std::vector<glm::vec3> kuiper;
        kuiper.reserve(static_cast<size_t>(kuiperCount));

        for (int i = 0; i < kuiperCount; ++i) {
            float radius = 52.0f + 16.0f * u01(rng);
            float angle = TWO_PI * u01(rng);
            float thickness = (u01(rng) - 0.5f) * 2.4f;
            float jitter = 0.9f * (u01(rng) - 0.5f);
            kuiper.push_back(glm::vec3((radius + jitter) * std::cos(angle), thickness, (radius + jitter) * std::sin(angle)));
        }

        kuiperBeltCount_ = static_cast<GLsizei>(kuiper.size());
        glGenVertexArrays(1, &kuiperBeltVao_);
        glGenBuffers(1, &kuiperBeltVbo_);
        glBindVertexArray(kuiperBeltVao_);
        glBindBuffer(GL_ARRAY_BUFFER, kuiperBeltVbo_);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(kuiper.size() * sizeof(glm::vec3)),
                     kuiper.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    glm::vec3 ComputeOrbitalPosition(const CelestialBody& body) const {
        if (body.orbitRadius <= 0.0f) {
            return glm::vec3(0.0f);
        }

        const float a = body.orbitRadius;
        const float e = glm::clamp(body.eccentricity, 0.0f, 0.95f);
        const float b = a * std::sqrt(1.0f - e * e);

        glm::vec3 pos(a * std::cos(body.orbitAngle), 0.0f, b * std::sin(body.orbitAngle));
        glm::mat4 inclination = glm::rotate(glm::mat4(1.0f), glm::radians(body.orbitInclinationDeg), glm::vec3(1.0f, 0.0f, 0.0f));
        return glm::vec3(inclination * glm::vec4(pos, 1.0f));
    }

    glm::vec3 CurrentCameraPosition() const {
        return glm::vec3(
            cameraTarget_.x + cameraDistance_ * std::cos(glm::radians(cameraPitch_)) * std::cos(glm::radians(cameraYaw_)),
            cameraTarget_.y + cameraDistance_ * std::sin(glm::radians(cameraPitch_)),
            cameraTarget_.z + cameraDistance_ * std::cos(glm::radians(cameraPitch_)) * std::sin(glm::radians(cameraYaw_)));
    }

    void ComputeWorldPositions() {
        if (worldPositions_.size() != bodies_.size()) {
            worldPositions_.assign(bodies_.size(), glm::vec3(0.0f));
        }

        for (size_t i = 0; i < bodies_.size(); ++i) {
            glm::vec3 local = ComputeOrbitalPosition(bodies_[i]);
            if (bodies_[i].parentIndex >= 0) {
                worldPositions_[i] = worldPositions_[static_cast<size_t>(bodies_[i].parentIndex)] + local;
            } else {
                worldPositions_[i] = local;
            }
        }
    }

    void HandleInput(float dt) {
        const float rotateSpeed = 70.0f;
        const float zoomSpeed = 28.0f;

        if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS) {
            cameraPitch_ += rotateSpeed * dt;
        }
        if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS) {
            cameraPitch_ -= rotateSpeed * dt;
        }
        if (glfwGetKey(window_, GLFW_KEY_LEFT) == GLFW_PRESS) {
            cameraYaw_ -= rotateSpeed * dt;
        }
        if (glfwGetKey(window_, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            cameraYaw_ += rotateSpeed * dt;
        }
        if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
            desiredCameraDistance_ -= zoomSpeed * dt;
        }
        if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
            desiredCameraDistance_ += zoomSpeed * dt;
        }
        if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
            timeScaleDaysPerSecond_ = glm::max(0.5f, timeScaleDaysPerSecond_ - 8.0f * dt);
        }
        if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
            timeScaleDaysPerSecond_ = glm::min(180.0f, timeScaleDaysPerSecond_ + 8.0f * dt);
        }

        int spaceState = glfwGetKey(window_, GLFW_KEY_SPACE);
        if (spaceState == GLFW_PRESS && !spacePressedLastFrame_) {
            paused_ = !paused_;
        }
        spacePressedLastFrame_ = (spaceState == GLFW_PRESS);

        int rState = glfwGetKey(window_, GLFW_KEY_R);
        if (rState == GLFW_PRESS && !rPressedLastFrame_) {
            ClearSelection();
        }
        rPressedLastFrame_ = (rState == GLFW_PRESS);

        int bState = glfwGetKey(window_, GLFW_KEY_B);
        if (bState == GLFW_PRESS && !bPressedLastFrame_) {
            beltsVisible_ = !beltsVisible_;
            std::cout << "Belts: " << (beltsVisible_ ? "ON" : "OFF") << "\n";
        }
        bPressedLastFrame_ = (bState == GLFW_PRESS);

        int leftState = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT);
        if (leftState == GLFW_PRESS && !leftMousePressedLastFrame_) {
            glfwGetCursorPos(window_, &queuedPickX_, &queuedPickY_);
            pickQueued_ = true;
        }
        leftMousePressedLastFrame_ = (leftState == GLFW_PRESS);

        int rightState = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT);
        if (rightState == GLFW_PRESS && !rightMousePressedLastFrame_) {
            ClearSelection();
        }
        rightMousePressedLastFrame_ = (rightState == GLFW_PRESS);

        if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window_, GLFW_TRUE);
        }
        


        cameraPitch_ = glm::clamp(cameraPitch_, -85.0f, 85.0f);
        desiredCameraDistance_ = glm::clamp(desiredCameraDistance_, 3.0f, 180.0f);
    }

    void Update(float dt) {
        if (!paused_) {
            float daysAdvanced = dt * timeScaleDaysPerSecond_;
            float hoursAdvanced = daysAdvanced * 24.0f;

            for (CelestialBody& body : bodies_) {
                if (body.orbitalPeriodDays > 0.0f && body.orbitRadius > 0.0f) {
                    body.orbitAngle += TWO_PI * (daysAdvanced / body.orbitalPeriodDays);
                    if (body.orbitAngle > TWO_PI) {
                        body.orbitAngle = std::fmod(body.orbitAngle, TWO_PI);
                    }
                }

                if (std::abs(body.rotationPeriodHours) > 0.001f) {
                    body.spinAngle += TWO_PI * (hoursAdvanced / body.rotationPeriodHours) * spinSpeedScale_;
                    if (std::abs(body.spinAngle) > TWO_PI) {
                        body.spinAngle = std::fmod(body.spinAngle, TWO_PI);
                    }
                }
            }
        }

        ComputeWorldPositions();
        ProcessQueuedPick();

        if (selectedBodyIndex_ >= 0 && selectedBodyIndex_ < static_cast<int>(bodies_.size())) {
            const CelestialBody& selected = bodies_[static_cast<size_t>(selectedBodyIndex_)];
            desiredCameraTarget_ = worldPositions_[static_cast<size_t>(selectedBodyIndex_)];
            desiredCameraDistance_ = glm::clamp(selected.visualRadius * 8.0f, 3.5f, 34.0f);
        }

        float cameraLerp = 1.0f - std::exp(-4.8f * dt);
        cameraTarget_ = glm::mix(cameraTarget_, desiredCameraTarget_, cameraLerp);
        cameraDistance_ = glm::mix(cameraDistance_, desiredCameraDistance_, cameraLerp);
    }

    void ProcessQueuedPick() {
        if (!pickQueued_) {
            return;
        }
        pickQueued_ = false;

        if (width_ <= 0 || height_ <= 0) {
            return;
        }

        glm::mat4 projection = glm::perspective(
            glm::radians(50.0f),
            static_cast<float>(width_) / static_cast<float>(glm::max(height_, 1)),
            0.1f,
            650.0f);
        glm::vec3 camPos = CurrentCameraPosition();
        glm::mat4 view = glm::lookAt(camPos, cameraTarget_, glm::vec3(0.0f, 1.0f, 0.0f));

        float x = static_cast<float>((2.0 * queuedPickX_) / static_cast<double>(width_) - 1.0);
        float y = static_cast<float>(1.0 - (2.0 * queuedPickY_) / static_cast<double>(height_));

        glm::vec4 rayClip(x, y, -1.0f, 1.0f);
        glm::vec4 rayEye = glm::inverse(projection) * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
        glm::vec3 rayDir = glm::normalize(glm::vec3(glm::inverse(view) * rayEye));

        int pickedIndex = -1;
        float bestT = std::numeric_limits<float>::max();

        for (size_t i = 0; i < bodies_.size(); ++i) {
            if (bodies_[i].name == "Moon") {
                continue;
            }

            const glm::vec3 center = worldPositions_[i];
            float radius = bodies_[i].visualRadius * 1.18f;
            glm::vec3 oc = camPos - center;

            float a = glm::dot(rayDir, rayDir);
            float b = 2.0f * glm::dot(oc, rayDir);
            float c = glm::dot(oc, oc) - radius * radius;
            float disc = b * b - 4.0f * a * c;
            if (disc < 0.0f) {
                continue;
            }

            float sqrtDisc = std::sqrt(disc);
            float t0 = (-b - sqrtDisc) / (2.0f * a);
            float t1 = (-b + sqrtDisc) / (2.0f * a);
            float t = (t0 > 0.0f) ? t0 : t1;
            if (t > 0.0f && t < bestT) {
                bestT = t;
                pickedIndex = static_cast<int>(i);
            }
        }

        if (pickedIndex >= 0) {
            SelectBody(pickedIndex);
        }
    }

    void SelectBody(int bodyIndex) {
        selectedBodyIndex_ = bodyIndex;
        const CelestialBody& body = bodies_[static_cast<size_t>(bodyIndex)];

        std::cout << "Selected: " << body.name << "\n";
        std::cout << body.description << "\n";
    }

    void ClearSelection() {
        selectedBodyIndex_ = -1;
        desiredCameraTarget_ = glm::vec3(0.0f);
        desiredCameraDistance_ = 72.0f;
    }

    void Render() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(
            glm::radians(50.0f),
            static_cast<float>(width_) / static_cast<float>(glm::max(height_, 1)),
            0.1f,
            650.0f);

        glm::vec3 cameraPos = CurrentCameraPosition();
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget_, glm::vec3(0.0f, 1.0f, 0.0f));

        DrawStars(view, projection);
        if (beltsVisible_) {
            DrawBelts(view, projection);
        }
        DrawOrbits(view, projection, worldPositions_);
        DrawBodies(view, projection, cameraPos, worldPositions_);
        DrawOverlayDescription();

        glfwSwapBuffers(window_);
    }

    void DrawPoints(GLuint vao,
                    GLsizei count,
                    const glm::mat4& view,
                    const glm::mat4& projection,
                    const glm::vec3& color,
                    float size,
                    bool depthTest) const {
        if (count <= 0) {
            return;
        }

        if (!depthTest) {
            glDisable(GL_DEPTH_TEST);
        }

        glUseProgram(pointProgram_);
        glUniformMatrix4fv(glGetUniformLocation(pointProgram_, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(pointProgram_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(pointProgram_, "pointColor"), 1, glm::value_ptr(color));
        glUniform1f(glGetUniformLocation(pointProgram_, "pointSize"), size);

        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, count);
        glBindVertexArray(0);

        if (!depthTest) {
            glEnable(GL_DEPTH_TEST);
        }
    }

    void DrawStars(const glm::mat4& view, const glm::mat4& projection) const {
        DrawPoints(starVao_, starCount_, view, projection, glm::vec3(1.0f, 1.0f, 1.0f), 1.8f, false);
    }

    void DrawBelts(const glm::mat4& view, const glm::mat4& projection) const {
        DrawPoints(asteroidBeltVao_, asteroidBeltCount_, view, projection, glm::vec3(0.70f, 0.62f, 0.54f), 2.2f, true);
        DrawPoints(kuiperBeltVao_, kuiperBeltCount_, view, projection, glm::vec3(0.56f, 0.67f, 0.76f), 1.9f, true);
    }

    void PushGlyphPoints(char glyph,
                         float left,
                         float top,
                         float pixelSize,
                         std::vector<glm::vec3>& points) const {
        const auto pattern = GlyphPattern(glyph);
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                if (pattern[static_cast<size_t>(row)] & (1u << (4 - col))) {
                    float px = left + static_cast<float>(col) * pixelSize;
                    float py = top + static_cast<float>(row) * pixelSize;
                    float ndcX = (px / static_cast<float>(width_)) * 2.0f - 1.0f;
                    float ndcY = 1.0f - (py / static_cast<float>(height_)) * 2.0f;
                    points.emplace_back(ndcX, ndcY, 0.0f);
                }
            }
        }
    }

    void DrawOverlayDescription() const {
        std::vector<std::string> lines;
        if (selectedBodyIndex_ >= 0 && selectedBodyIndex_ < static_cast<int>(bodies_.size())) {
            const CelestialBody& body = bodies_[static_cast<size_t>(selectedBodyIndex_)];
            lines.push_back("FOCUS: " + ToUpperAscii(body.name));
            const auto wrapped = WrapText(ToUpperAscii(body.description), 52);
            lines.insert(lines.end(), wrapped.begin(), wrapped.end());
        } else {
            lines.push_back("CLICK A PLANET TO VIEW DESCRIPTION");
        }

        lines.push_back(beltsVisible_ ? "BELTS: ON (PRESS B TO TOGGLE)" : "BELTS: OFF (PRESS B TO TOGGLE)");

        std::vector<glm::vec3> textPoints;
        std::vector<glm::vec3> shadowPoints;
        textPoints.reserve(10000);
        shadowPoints.reserve(10000);

        const float fontPixel = 2.8f;
        const float lineHeight = 10.0f * fontPixel;
        float startX = 16.0f;
        float startY = 22.0f;

        for (size_t li = 0; li < lines.size(); ++li) {
            float cursorX = startX;
            float y = startY + static_cast<float>(li) * lineHeight;
            for (char ch : lines[li]) {
                char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
                PushGlyphPoints(upper, cursorX + 1.3f, y + 1.3f, fontPixel, shadowPoints);
                PushGlyphPoints(upper, cursorX, y, fontPixel, textPoints);
                cursorX += 6.0f * fontPixel;
            }
        }

        glm::mat4 identity(1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (!shadowPoints.empty()) {
            GLuint tempVao = 0;
            GLuint tempVbo = 0;
            glGenVertexArrays(1, &tempVao);
            glGenBuffers(1, &tempVbo);
            glBindVertexArray(tempVao);
            glBindBuffer(GL_ARRAY_BUFFER, tempVbo);
            glBufferData(GL_ARRAY_BUFFER,
                         static_cast<GLsizeiptr>(shadowPoints.size() * sizeof(glm::vec3)),
                         shadowPoints.data(),
                         GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
            glEnableVertexAttribArray(0);

            glUseProgram(pointProgram_);
            glUniformMatrix4fv(glGetUniformLocation(pointProgram_, "view"), 1, GL_FALSE, glm::value_ptr(identity));
            glUniformMatrix4fv(glGetUniformLocation(pointProgram_, "projection"), 1, GL_FALSE, glm::value_ptr(identity));
            glUniform3f(glGetUniformLocation(pointProgram_, "pointColor"), 0.03f, 0.03f, 0.03f);
            glUniform1f(glGetUniformLocation(pointProgram_, "pointSize"), 2.8f);
            glDisable(GL_DEPTH_TEST);
            glBindVertexArray(tempVao);
            glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(shadowPoints.size()));
            glBindVertexArray(0);

            glDeleteBuffers(1, &tempVbo);
            glDeleteVertexArrays(1, &tempVao);
        }

        if (!textPoints.empty()) {
            GLuint tempVao = 0;
            GLuint tempVbo = 0;
            glGenVertexArrays(1, &tempVao);
            glGenBuffers(1, &tempVbo);
            glBindVertexArray(tempVao);
            glBindBuffer(GL_ARRAY_BUFFER, tempVbo);
            glBufferData(GL_ARRAY_BUFFER,
                         static_cast<GLsizeiptr>(textPoints.size() * sizeof(glm::vec3)),
                         textPoints.data(),
                         GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
            glEnableVertexAttribArray(0);

            glUseProgram(pointProgram_);
            glUniformMatrix4fv(glGetUniformLocation(pointProgram_, "view"), 1, GL_FALSE, glm::value_ptr(identity));
            glUniformMatrix4fv(glGetUniformLocation(pointProgram_, "projection"), 1, GL_FALSE, glm::value_ptr(identity));
            glUniform3f(glGetUniformLocation(pointProgram_, "pointColor"), 0.95f, 0.95f, 0.92f);
            glUniform1f(glGetUniformLocation(pointProgram_, "pointSize"), 2.8f);
            glBindVertexArray(tempVao);
            glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(textPoints.size()));
            glBindVertexArray(0);

            glDeleteBuffers(1, &tempVbo);
            glDeleteVertexArrays(1, &tempVao);
        }

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }

    void DrawOrbits(const glm::mat4& view,
                    const glm::mat4& projection,
                    const std::vector<glm::vec3>& worldPositions) const {
        glUseProgram(lineProgram_);
        glUniformMatrix4fv(glGetUniformLocation(lineProgram_, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lineProgram_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(orbitVao_);

        for (size_t i = 0; i < bodies_.size(); ++i) {
            const CelestialBody& body = bodies_[i];
            if (!body.drawOrbit || body.orbitRadius <= 0.0f) {
                continue;
            }

            float e = glm::clamp(body.eccentricity, 0.0f, 0.95f);
            float b = std::sqrt(1.0f - e * e);

            glm::mat4 model(1.0f);
            if (body.parentIndex >= 0) {
                model = glm::translate(model, worldPositions[static_cast<size_t>(body.parentIndex)]);
            }
            model = glm::rotate(model, glm::radians(body.orbitInclinationDeg), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(body.orbitRadius, 1.0f, body.orbitRadius * b));

            glUniformMatrix4fv(glGetUniformLocation(lineProgram_, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(glGetUniformLocation(lineProgram_, "lineColor"), 0.34f, 0.38f, 0.46f);

            glDrawArrays(GL_LINE_LOOP, 0, kOrbitSegments);
        }

        glBindVertexArray(0);
    }

    void DrawBodies(const glm::mat4& view,
                    const glm::mat4& projection,
                    const glm::vec3& cameraPos,
                    const std::vector<glm::vec3>& worldPositions) const {
        glUseProgram(bodyProgram_);

        glUniformMatrix4fv(glGetUniformLocation(bodyProgram_, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(bodyProgram_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(bodyProgram_, "lightPos"), 0.0f, 0.0f, 0.0f);
        glUniform3fv(glGetUniformLocation(bodyProgram_, "viewPos"), 1, glm::value_ptr(cameraPos));

        for (size_t i = 0; i < bodies_.size(); ++i) {
            const CelestialBody& body = bodies_[i];

            glm::mat4 model(1.0f);
            model = glm::translate(model, worldPositions[i]);
            model = glm::rotate(model, glm::radians(body.axialTiltDeg), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, body.spinAngle, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(body.visualRadius));

            float ambient = (selectedBodyIndex_ == static_cast<int>(i)) ? 0.15f : 0.08f;

            glUniformMatrix4fv(glGetUniformLocation(bodyProgram_, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(glGetUniformLocation(bodyProgram_, "baseColor"), 1, glm::value_ptr(body.color));
            glUniform1f(glGetUniformLocation(bodyProgram_, "ambientStrength"), ambient);
            glUniform1f(glGetUniformLocation(bodyProgram_, "emissiveStrength"), body.emissive);
            glUniform1f(glGetUniformLocation(bodyProgram_, "earthMask"), body.name == "Earth" ? 1.0f : 0.0f);
            glUniform1f(glGetUniformLocation(bodyProgram_, "earthPhase"), body.spinAngle);

            body.sphere->Draw();

            if (body.hasRing && saturnRing_) {
                glm::mat4 ringModel(1.0f);
                ringModel = glm::translate(ringModel, worldPositions[i]);
                ringModel = glm::rotate(ringModel, glm::radians(body.axialTiltDeg), glm::vec3(0.0f, 0.0f, 1.0f));
                ringModel = glm::scale(ringModel, glm::vec3(body.visualRadius));

                glUniformMatrix4fv(glGetUniformLocation(bodyProgram_, "model"), 1, GL_FALSE, glm::value_ptr(ringModel));
                glUniform3f(glGetUniformLocation(bodyProgram_, "baseColor"), 0.85f, 0.80f, 0.70f);
                glUniform1f(glGetUniformLocation(bodyProgram_, "ambientStrength"), 0.16f);
                glUniform1f(glGetUniformLocation(bodyProgram_, "emissiveStrength"), 0.0f);
                glUniform1f(glGetUniformLocation(bodyProgram_, "earthMask"), 0.0f);
                glUniform1f(glGetUniformLocation(bodyProgram_, "earthPhase"), 0.0f);
                saturnRing_->Draw();
            }
        }
    }

private:
    static constexpr int kOrbitSegments = 220;

    GLFWwindow* window_ = nullptr;
    int width_ = 1280;
    int height_ = 800;

    GLuint bodyProgram_ = 0;
    GLuint lineProgram_ = 0;
    GLuint pointProgram_ = 0;

    GLuint orbitVao_ = 0;
    GLuint orbitVbo_ = 0;

    GLuint starVao_ = 0;
    GLuint starVbo_ = 0;
    GLsizei starCount_ = 0;

    GLuint asteroidBeltVao_ = 0;
    GLuint asteroidBeltVbo_ = 0;
    GLsizei asteroidBeltCount_ = 0;

    GLuint kuiperBeltVao_ = 0;
    GLuint kuiperBeltVbo_ = 0;
    GLsizei kuiperBeltCount_ = 0;

    RingMesh* saturnRing_ = nullptr;
    std::vector<CelestialBody> bodies_;
    std::vector<glm::vec3> worldPositions_;

    float cameraDistance_ = 72.0f;
    float desiredCameraDistance_ = 72.0f;
    float cameraYaw_ = 30.0f;
    float cameraPitch_ = 22.0f;
    glm::vec3 cameraTarget_ = glm::vec3(0.0f);
    glm::vec3 desiredCameraTarget_ = glm::vec3(0.0f);

    int selectedBodyIndex_ = -1;
    bool pickQueued_ = false;
    double queuedPickX_ = 0.0;
    double queuedPickY_ = 0.0;

    float timeScaleDaysPerSecond_ = 28.0f;
    float spinSpeedScale_ = 0.0035f;
    bool paused_ = false;
    bool beltsVisible_ = true;
    bool spacePressedLastFrame_ = false;
    bool rPressedLastFrame_ = false;
    bool bPressedLastFrame_ = false;
    bool leftMousePressedLastFrame_ = false;
    bool rightMousePressedLastFrame_ = false;
};
}

int main() {
    SolarSystem system;
    if (!system.Initialize(1280, 800)) {
        return -1;
    }

    system.Run();
    system.Shutdown();
    return 0;
}
