#ifndef READCONTROL_H
#define READCONTROL_H

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <getopt.h>
#include "../lib/vgl.h"
#include "../lib/glm/glm.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"
#include "../lib/glm/gtc/type_ptr.hpp"
#include "../lib/glm/gtx/rotate_vector.hpp"
#include "../lib/SOIL.h"
#include "../lib/LoadShaders.h"

void processOpts(int argc,char *argv[]);
void usage(std::ostream &file);
GLfloat getScaleFactor();

void loadOBJ(
        std::string filePath,
        std::vector<glm::vec4> &vertices,
        std::vector<glm::vec2> &textures,
        std::vector<glm::vec3> &normals,
        std::vector<glm::vec3> &ka,
        std::vector<glm::vec3> &kd,
        std::vector<glm::vec3> &ks,
        std::vector<GLfloat> &ns,
        std::vector<int> &textureMapOffset);

glm::vec4 vertex(std::istringstream &input);
glm::vec3 normal(std::istringstream &input);
glm::vec2 text(std::istringstream &input);
void parseView(std::istringstream &iss);
void parseLight(std::istringstream &iss);


void face(std::istringstream &input,
          std::vector<GLuint> &vFaces,
          std::vector<GLuint>&nFaces,
          std::vector<GLuint>&tFaces,
          std::vector<glm::vec3>&ka,
          std::vector<glm::vec3>&kd,
          std::vector<glm::vec3> &ks,
          std::vector<GLfloat> &ns,
          std::vector<int> &textureMapOffset);

void calculateNormals(std::vector<GLuint> &vFaces,
                      std::vector<glm::vec3> &normals,
                      std::vector<glm::vec4> &vertices);

void setupMTL(std::istringstream &iss);
void readControlFile(std::string controlFile);
void setCurrentColors(std::istringstream &iss);

void commitCurrentObj(glm::mat4 modelMatrix,
                      std::vector<glm::vec4> &vertices,
                      std::vector<glm::vec3> &normals,
                      std::vector<glm::vec2> &textureVertices,
                      std::vector<glm::vec3> &ka,
                      std::vector<glm::vec3> &kd,
                      std::vector<glm::vec3> &ks,
                      std::vector<GLfloat> &ns,
                      std::vector<int> &textureMapOffset);

void setupView(glm::vec3 camera, glm::vec3 focal, glm::vec3 viewUpper);


struct MTLProperties {
    std::string name;
    glm::vec3 Ka;
    glm::vec3 Kd;
    glm::vec3 Ks;
    GLfloat Ns;
    int textureOffset;
};

struct LightProperties {
    bool isEnabled;
    bool isLocal;
    bool isSpot;
    glm::vec3 ambient;
    glm::vec3 color;
    glm::vec3 position;
    glm::vec3 halfVector;
    glm::vec3 coneDirection;
    GLfloat spotCosCutoff;
    GLfloat spotExponent;
    GLfloat constantAttenuation;
    GLfloat linearAttenuation;
    GLfloat quadraticAttenuation;
};

void addLight(LightProperties light);
void setupLights();

int addTexture(char* fileName, bool invertY);
bool loadTexture(char* fileName, bool invertY);

#endif
