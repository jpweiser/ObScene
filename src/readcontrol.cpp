
#include "readcontrol.h"

#define NUM_VERTICES_PER_FACE 3

using namespace std;

glm::vec3 currentKA;
glm::vec3 currentKD;
glm::vec3 currentKS;
GLfloat currentNS;
GLuint currentMKDOffset;

vector<MTLProperties> materials;

string filePath = "";

vector<glm::vec4> vertices;
vector<glm::vec2> textureVertices;
vector<glm::vec3> normals;

vector<glm::vec3> ka;
vector<glm::vec3> kd;
vector<glm::vec3> ks;
vector<GLfloat> ns;

vector<int> textureMapOffset;
/////////////////////////////////////////////////////////////////////////////
// readControlFile
///////////////////////////////////////////////////////////////////////////
void readControlFile(string controlFile) {
    ifstream inputFile(controlFile.c_str(), ios::in);
    if (!inputFile) {
        cerr << "Error: Invalid command or filename " + controlFile << endl;
        usage(cerr);
    }

    bool isFirst = true;

    // Model matrix
    glm::mat4 modelMatrix;;



    string line;
    while (getline(inputFile, line)) {

        string command;
        istringstream iss(line);

        while (iss >> command) { // read whole line!

            if (command == "obj") {
                if (!isFirst) {
                    commitCurrentObj(modelMatrix,
                                     vertices,
                                     normals,
                                     textureVertices,
                                     ka, kd, ks, ns,
                                     textureMapOffset);
                }
                isFirst = false;
                modelMatrix = glm::mat4(1.0f);
                string objFile;
                iss >> objFile;
                loadOBJ(objFile,
                        vertices,
                        textureVertices,
                        normals,
                        ka, kd, ks, ns,
                        textureMapOffset);
            } else if (command == "rx") {
                // rotate model matrix
                GLfloat degrees;
                iss >> degrees;
                modelMatrix = glm::rotate(modelMatrix, degrees * 3.14159f / 180.f, glm::vec3(1.f, 0.f, 0.f));
            } else if (command == "ry") {
                // rotate model matrix
                float degrees;
                iss >> degrees;
                modelMatrix = glm::rotate(modelMatrix, degrees * 3.14159f / 180.f, glm::vec3(0.f, 0.f, 1.f));
            } else if (command == "rz") {
                // rotate model matrix
                GLfloat degrees;
                iss >> degrees;
                modelMatrix = glm::rotate(modelMatrix, degrees * 3.14159f / 180.f, glm::vec3(0.f, 1.f, 0.f));
            } else if (command == "s") {
                // scale model matrix
                GLfloat x,y,z;
                iss >> x >> y >> z;
                modelMatrix = glm::scale(modelMatrix, glm::vec3(x,y,z));
            } else if (command == "t") {
                // translate model matrix
                GLfloat x,y,z;
                iss >> x >> y >> z;
                modelMatrix[3][0] += x;
                modelMatrix[3][1] += y;
                modelMatrix[3][2] += z;
            } else if (command == "light") {
                parseLight(iss);
            } else if (command == "view") {
                parseView(iss);
            }

        }
    }


    commitCurrentObj(modelMatrix,
                     vertices,
                     normals,
                     textureVertices,
                     ka, kd, ks, ns,
                     textureMapOffset);

} // readControlFile

void parseLight(istringstream &iss) {
    string command;
    LightProperties light;

    light.isEnabled = true;
    light.isLocal = false; // by default
    light.isSpot = false; // by default

    while (iss >> command) {
        if (command == "local") {
            light.isLocal = true;
        } else if (command == "spot") {
            light.isLocal = true;
            light.isSpot = true;
        } else if (command == "directional") {
            continue;
        } else if (command == "ambient") {
            GLfloat r,g,b;
            iss >> r >> g >> b;
            light.ambient = glm::vec3(r,g,b);
        } else if (command == "color") {
            GLfloat r,g,b;
            iss >> r >> g >> b;
            light.color = glm::vec3(r,g,b);
        } else if (command == "position") {
            GLfloat x,y,z;
            iss >> x >> y >> z;
            light.position = glm::vec3(x,y,z);
        } else if (command == "constAtt") {
            GLfloat ca;
            iss >> ca;
            light.constantAttenuation = ca;
        } else if (command == "linearAtt") {
            GLfloat la;
            iss >> la;
            light.linearAttenuation = la;
        } else if (command == "quadAtt") {
            GLfloat qa;
            iss >> qa;
            light.quadraticAttenuation = qa;
        } else if (command == "coneDirection") {
            GLfloat x,y,z;
            iss >> x >> y >> z;
            light.coneDirection = glm::vec3(x,y,z);
        } else if (command == "spotCosCutoff") {
            GLfloat cutoff;
            iss >> cutoff;
            light.spotCosCutoff = cutoff;
        } else if (command == "spotExponent") {
            GLfloat ex;
            iss >> ex;
            light.spotExponent = ex;
        } else {
            cout << "Unknown Command: " + command;
        }
    }

    addLight(light);
}

void parseView(istringstream &iss) {
    string command;
    glm::vec3 camera, focal, viewup;
    while (iss >> command) {
        GLfloat x,y,z;
        iss >> x >> y >> z;
        if (command == "camera") {
            camera = glm::vec3(x,y,z);
        } else if (command == "focal") {
            focal = glm::vec3(x,y,z);
        } else if (command == "viewup") {
            viewup = glm::vec3(x,y,z);
        }
    }

    setupView(camera,focal, viewup);
}


/////////////////////////////////////////////////////////////////////////////
// loadOBJ
///////////////////////////////////////////////////////////////////////////

void loadOBJ(
        string fileName,
        vector<glm::vec4> &vertices,
        vector<glm::vec2> &textures,
        vector<glm::vec3> &normals,
        vector<glm::vec3> &ka,
        vector<glm::vec3> &kd,
        vector<glm::vec3> &ks,
        vector<GLfloat> &ns,
        vector<int> &textureMapOffset
){
    ifstream inputFile(fileName.c_str(), ios::in);
    if (!inputFile) {
        cerr << "Cannot locate " + fileName << endl;
        usage(cerr);
    }

    if (fileName.find_last_of("/") != fileName.npos) {
        filePath = fileName.substr(0, fileName.find_last_of("/") + 1);
    }

    vector<glm::vec4> tempVertices;
    vector<glm::vec3> tempNormals;
    vector<glm::vec2> tempTextures;
    vector<GLuint> vFaces;
    vector<GLuint> nFaces;
    vector<GLuint>tFaces;

    // default color values
    currentKA = glm::vec3(0.1f, 0.1f, 0.1f);
    currentKD = glm::vec3(0.9f, 0.9f, 0.9f);
    currentKS = glm::vec3(1.0f, 1.0f, 1.0f);
    currentNS = 60.f;
    currentMKDOffset = 0;

    string line;
    while (getline(inputFile, line)) {

        string command;
        istringstream iss(line);

        iss >> command;

        if (command == "v") {
            tempVertices.push_back(vertex(iss));
        } else if (command == "vn") {
            tempNormals.push_back(normal(iss));
        } else if (command == "vt") {
            tempTextures.push_back(text(iss));
        } else if (command == "f") {
            face(iss, vFaces, nFaces, tFaces, ka, kd, ks, ns, textureMapOffset);
        } else if (command == "mtllib") {
            setupMTL(iss);
        } else if (command == "usemtl") {
            setCurrentColors(iss);
        }
    }


    if (tempNormals.size() == 0) {
         calculateNormals(vFaces, normals, tempVertices);
    } else {
        for (int i = 0; i < nFaces.size(); i++) {
            normals.push_back (tempNormals[nFaces[i]]);
        }
    }

   for (int i = 0; i < vFaces.size(); i++) {

       vertices.push_back (tempVertices[vFaces[i]]);
   }

   for (int i = 0; i < tFaces.size(); i++) {
       textures.push_back (tempTextures[tFaces[i]]);
   }
} // loadOBJ

//////////////////////////////////////////////////////////////////////////
// vertex
///////////////////////////////////////////////////////////////////////

glm::vec4 vertex(istringstream &iss) {
    GLfloat x,y,z, w;
    iss >> x; iss >> y; iss>>z;

    if (!(iss >> w))
        w = 1.0;

    return glm::vec4(x,y,z,w);

} // vertex

/////////////////////////////////////////////////////////////////////////
// normal
///////////////////////////////////////////////////////////////////////

glm::vec3 normal(istringstream &iss) {
    GLfloat x,y,z;
    iss >> x; iss >> y; iss>>z;
    return glm::vec3(x,y,z);

} // normal

////////////////////////////////////////////////////////////////////////
// text
//////////////////////////////////////////////////////////////////////

glm::vec2 text(istringstream &iss) {
    GLfloat x,y;
    iss >> x; iss >> y;
    return glm::vec2(x,y);

} // text

///////////////////////////////////////////////////////////////////////////
// face
/////////////////////////////////////////////////////////////////////////

void face(istringstream &iss,
          vector<GLuint> &vFaces,
          vector<GLuint>&nFaces,
          vector<GLuint>&tFaces,
          vector<glm::vec3> &ka,
          vector<glm::vec3> &kd,
          vector<glm::vec3> &ks,
          vector<GLfloat> &ns,
          vector<int> &textureMapOffset)
           {

    GLuint vert, text, norm;

    for (int i = 0; i < NUM_VERTICES_PER_FACE; i++) {

        iss >> vert;

        vFaces.push_back(--vert);
        ka.push_back (currentKA);
        kd.push_back (currentKD);
        ks.push_back (currentKS);
        ns.push_back (currentNS);
        textureMapOffset.push_back (currentMKDOffset);



    // may have slashes, if so deal with them

    // lots of help from http://stackoverflow.com/questions/1894886/parsing-a-comma-delimited-stdstring
        if( '/' == iss.peek() ){

            iss.ignore(1,'/');

        // this one accounts for optional texture coords
            if( isdigit(iss.peek()) ){

                iss >> text;
                tFaces.push_back(--text);

            }
            // this one accounts for normals
            if( '/' == iss.peek() ){

                iss.ignore(1,'/');
                iss >> norm;
                nFaces.push_back(--norm);
            }
        }
    }
} //face

//////////////////////////////////////////////////////////////////////////
// calculateNormals
////////////////////////////////////////////////////////////////////////

void calculateNormals(vector<GLuint> &vFaces,
                      vector<glm::vec3> &normals,
                      vector<glm::vec4> &vertices) {

    for (int i = 0; i < vFaces.size(); i+=3) {

        GLuint vertex = vFaces[i];
        GLuint pointA = vFaces[i+1];
        GLuint pointB = vFaces[i+2];

        glm::vec3 firstLine = glm::vec3(vertices[pointA]) - glm::vec3(vertices[vertex]);
        glm::vec3 secondLine = glm::vec3(vertices[pointB]) - glm::vec3(vertices[vertex]);

        glm::vec3 normal = glm::normalize(glm::cross(firstLine, secondLine));

        for (int j = 0; j < NUM_VERTICES_PER_FACE; j++) {
            normals.push_back(normal);
        }
    }
} // calculateNormals

//////////////////////////////////////////////////////////////////////////////
// setupMTL
////////////////////////////////////////////////////////////////////////////

void setupMTL(istringstream &input) {

    string file;
    input >> file;
    string fileName = filePath + file;

    ifstream inputFile(fileName.c_str(), ios::in);
    if (!inputFile) {
        cerr << "Cannot locate " + fileName << endl;
        cerr << "Using default color" << endl;
        return;
    }

    MTLProperties current;
    current.Ka = glm::vec3(0.1f, 0.1f, 0.1f);
    current.Kd = glm::vec3(0.9f, 0.9f, 0.9f);
    current.Ks = glm::vec3(1.0f, 0.f, 0.f);
    current.Ns = 20.f;
    current.textureOffset = -1;

    MTLProperties defaultMTL = current;

    bool isFirst = true;

    string line;

    while (getline(inputFile, line)) {
        string command;
        istringstream iss(line);

        iss >> command;

        if (command == "newmtl") {
            if (!isFirst) {
                materials.push_back(current);
                current = defaultMTL;
            }
            isFirst = false;
            string name;
            iss >> name;
            current.name = name;
        } else if (command == "Ka") {
            GLfloat r,g,b;
            iss >> r >> g >> b;
            current.Ka = glm::vec3(r,g,b);
        } else if (command == "Kd") {
            GLfloat r,g,b;
            iss >> r >> g >> b;
            current.Kd = glm::vec3(r,g,b);
        } else if (command == "Ks") {
            GLfloat r,g,b;
            iss >> r >> g >> b;
            current.Ks = glm::vec3(r,g,b);
        } else if (command == "Ns") {
            GLfloat Ns;
            iss >> Ns;
            current.Ns = Ns;
        } else if (command == "map_Kd") {
            string mapFile;
            iss >> mapFile;
            current.textureOffset = addTexture(const_cast<char*>(mapFile.c_str()), true);

        }
    }
    materials.push_back(current);

} // setupMTL

/////////////////////////////////////////////////////////////////////////////
// setCurrentColors
//////////////////////////////////////////////////////////////////////////

void setCurrentColors(istringstream &iss) {

    string color;
    iss >> color;
    int i;
    for (i = 0; i < materials.size(); i++) {
        if (materials[i].name == color) {
            currentKA = materials[i].Ka;
            currentKD = materials[i].Kd;
            currentKS = materials[i].Ks;
            currentNS = materials[i].Ns;
            currentMKDOffset = materials[i].textureOffset;
            break;
        }
    }
} // setCurrentColors
