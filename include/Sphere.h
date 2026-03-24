#ifndef SPHERE_H
#define SPHERE_H

#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>
#include <cmath>
#include <cstddef>

class Sphere {
public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
    };

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;

    // Sphere bhaneko yesma stacks + sectors ho
    // Stacks = rings from bottom to top
    // Sectors = slices around the sphere

    Sphere(float radius = 1.0f, int sectorCount = 36, int stackCount = 18) {
        generateSphere(radius, sectorCount, stackCount);
        setupMesh();
    }

    ~Sphere() {
        glDeleteBuffers(1, &EBO);
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void Draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    void generateSphere(float radius, int sectorCount, int stackCount) {
        vertices.clear();
        indices.clear();

        float x, y, z, xy;
        float sectorStep = 2.0f * 3.14159f / sectorCount;  // yo chai angle for each sector side slice ho
        float stackStep = 3.14159f / stackCount;
        float sectorAngle, stackAngle;

        for (int i = 0; i <= stackCount; ++i) {
            stackAngle = 3.14159f / 2.0f - i * stackStep;
            xy = radius * cosf(stackAngle);
            z = radius * sinf(stackAngle);

            for (int j = 0; j <= sectorCount; ++j) {
                sectorAngle = j * sectorStep;
                x = xy * cosf(sectorAngle);
                y = xy * sinf(sectorAngle);

                glm::vec3 position(x, y, z);
                glm::vec3 normal = glm::normalize(position);
                vertices.push_back({position, normal});
            }
        }

        // OPENGL Le triangles ko form ma render garna cha, so 3 vertices ko group ma index haru push garna parcha to make sphere otherwise it cannot as it renders only traingle
        unsigned int k1, k2;
        for (int i = 0; i < stackCount; ++i) {
            k1 = i * (sectorCount + 1);
            k2 = k1 + sectorCount + 1;

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);

                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    // Yesle chai math lai CPU ko RAM bata Graphics Card ko memory ma halcha

    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};

#endif
