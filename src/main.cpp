// assignment 2
#include "readcontrol.h"


using namespace std;

// constants used for positions sent to shader program
const GLuint vPosition = 0;
const GLuint vNormal   = 1;
const GLuint vAmbient  = 2;
const GLuint vDiffuse  = 3;
const GLuint vSpecular = 4;
const GLuint vShiny    = 5;
const GLuint vTexture  = 6;

// the following five vectors must go together!
vector<GLuint> VAOS; // vertex array objects
vector<GLuint> numVertices; // number of vertices in each object
vector<glm::mat4> modelMatrices; // holds model matrices for each .obj file
vector<MTLProperties> mtlProperties; // holds material properties
vector<vector<glm::vec3> > texInfo; // keeps track of [number of verts with tex][texloc][isTextured]

GLuint classic; // shader program

glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
glm::vec3 lighting;

// View parameters
glm::vec3 eye;
glm::vec3 center;
glm::vec3 viewUp;

glm::vec3 defaultEye;
glm::vec3 defaultCenter;

// Keep references to shader variables
GLuint MVP_ID;
GLuint MV_ID;
GLuint Normal_ID;
GLuint Ka_ID;
GLuint Kd_ID;
GLuint Ks_ID;
GLuint Ns_ID;

GLuint LightingID; // figure this out for arrays

GLfloat height;
GLfloat width;

string controlFile;

bool wireframe = false;
bool viewSet = false;

vector<glm::vec3> centers;
glm::vec3 maxVec;

#define MAX_LIGHTS 4
int numLights = 0;
LightProperties Lights[MAX_LIGHTS];

string program;

/*********************************************/
GLint texLoc;
GLint numTextures = 0;
GLint texUnit = 0;
GLint isTextured;
/*********************************************/

/////////////////////////////////////////////////////
// loadTexture
/////////////////////////////////////////////////////
bool loadTexture(char* fileName, bool invertY)
{
  glActiveTexture(GL_TEXTURE0 + numTextures);
	unsigned char *image;
	int width, height, channels;

	image = SOIL_load_image(fileName, &width, &height, &channels, SOIL_LOAD_AUTO);
	if (image == NULL)
	{
		cerr << "Unable to load image: " << fileName << endl;
		return false;
	}
	else
	{
		if (invertY)
		{
			for (int j = 0; j * 2 < height; ++j)
			{
				int index1 = j * width * channels;
				int index2 = (height - 1 - j) * width * channels;
				for (int i = width * channels; i > 0; --i)
				{
					unsigned char temp = image[index1];
					image[index1] = image[index2];
					image[index2] = temp;
					++index1;
					++index2;
				}
			}
		}
		GLuint tex2;
		glGenTextures(1, &tex2);
		glBindTexture(GL_TEXTURE_2D, tex2);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		if (channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
			GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (channels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, image);

		SOIL_free_image_data(image);	// free up memory allocated by SOIL_load_image

		return true;
	}
}

int addTexture(char* fileName, bool invertY) {

  bool success = loadTexture(fileName, invertY);
  if (!success) {
  	cerr << "error reading texture " << numTextures << endl;
    return -1;
  }
  return numTextures++;
}

/****************************************
* Init
*
* Initialize environment
*****************************************/
void init(){

    glClearColor(0.0f, 0.1f, 0.3f, 0.1f);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    // Set up shader -- this is the only one required for this assignment
    ShaderInfo  shader[] = {
        { GL_VERTEX_SHADER, "shaders/classic.vert" },
        { GL_FRAGMENT_SHADER, "shaders/classic.frag" },
        { GL_NONE, NULL }
    };

    classic = LoadShaders( shader );

    // find variable in shader and connect
    MVP_ID = glGetUniformLocation(classic, "MVPMatrix");

    // Find in shader and connect
    MV_ID = glGetUniformLocation(classic, "MVMatrix");

    Normal_ID = glGetUniformLocation(classic, "NormalMatrix");

    texLoc = glGetUniformLocation(classic, "tex");

    isTextured = glGetUniformLocation(classic, "isTextured");

    maxVec = glm::vec3(0.f,0.f,0.f);

    for (int i = 0; i < MAX_LIGHTS; i++) {

        Lights[i].isEnabled = false;
	    Lights[i].isLocal = false;
    }

    ////////////////////////////////////////////////////////////////////////
    readControlFile(controlFile);
    //////////////////////////////////////////////////////////////////////

    setupLights();

    if (!viewSet) {
        // find center of all objects
        glm::vec3 sum = glm::vec3(0.f,0.f,0.f);
        for (int i = 0; i < centers.size(); i++) {
            sum += centers[i];
        }

        defaultEye = glm::vec3(
            1.75*maxVec.x,
            1.75*maxVec.y,
            1.75*maxVec.z
        );

        defaultCenter = glm::vec3(
            sum.x/centers.size(),
            sum.y/centers.size(),
            sum.z/centers.size()
        );

        // View matrix
        eye = defaultEye;
        center = defaultCenter;
        viewUp = glm::vec3( 0.f, 0.f, 1.f);
        viewMatrix=glm::lookAt( eye, center,viewUp);
    }

    //------------------------------------------------------------------------


} // init

void setupView(glm::vec3 camera, glm::vec3 focal, glm::vec3 viewUpper) {
    viewSet = true;
    eye = camera;
    defaultEye = eye;
    center = focal;
    defaultCenter = center;
    viewUp = viewUpper;
    viewMatrix=glm::lookAt( eye, center,viewUp);
}

void addLight(LightProperties light) {
    if (numLights < MAX_LIGHTS) {
        Lights[numLights++] = light;
    }
}

void setupLights() {
    for (int i = 0; i < numLights; i++) {
        ostringstream oss;
        oss << "Lights[" << i << "].";
        string light = oss.str();

        GLuint loc = glGetUniformLocation(classic, (light + "isEnabled").c_str());
        glProgramUniform1i(classic, loc, Lights[i].isEnabled);

        loc = glGetUniformLocation(classic, (light + "isLocal").c_str());
        glProgramUniform1i(classic, loc, Lights[i].isLocal);

        loc = glGetUniformLocation(classic, (light + "isSpot").c_str());
        glProgramUniform1i(classic, loc, Lights[i].isSpot);

        loc = glGetUniformLocation(classic, (light + "ambient").c_str());
        glProgramUniform3fv(classic, loc, 1, value_ptr(Lights[i].ambient));

        loc = glGetUniformLocation(classic, (light + "color").c_str());
        glProgramUniform3fv(classic, loc, 1, value_ptr(Lights[i].color));

        loc = glGetUniformLocation(classic, (light + "position").c_str());
        glProgramUniform3fv(classic, loc, 1, value_ptr(Lights[i].position));

        loc = glGetUniformLocation(classic, (light + "halfVector").c_str());
        glProgramUniform3fv(classic, loc, 1, value_ptr(Lights[i].halfVector));

        loc = glGetUniformLocation(classic, (light + "coneDirection").c_str());
        glProgramUniform3fv(classic, loc, 1, value_ptr(Lights[i].coneDirection));

        loc = glGetUniformLocation(classic, (light + "spotCosCutoff").c_str());
        glProgramUniform1f(classic, loc, Lights[i].spotCosCutoff);

        loc = glGetUniformLocation(classic, (light + "spotExponent").c_str());
        glProgramUniform1f(classic, loc, Lights[i].spotExponent);

        loc = glGetUniformLocation(classic, (light + "constantAttenuation").c_str());
        glProgramUniform1f(classic, loc, Lights[i].constantAttenuation);

        loc = glGetUniformLocation(classic, (light + "linearAttenuation").c_str());
        glProgramUniform1f(classic, loc, Lights[i].linearAttenuation);

        loc = glGetUniformLocation(classic, (light + "quadraticAttenuation").c_str());
        glProgramUniform1f(classic, loc, Lights[i].quadraticAttenuation);
    }
}



////////////////////////////////////////////////////////////////////
//	display
////////////////////////////////////////////////////////////////////
void display (void )
{

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram( classic );

    for (int i = 0; i < VAOS.size(); i++) {

        glBindVertexArray( VAOS[i] );

        glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrices[i];
        glm::mat4 MV = viewMatrix * modelMatrices[i];
        glm::mat3 normalMatrix = glm::mat3(MV);
        glUniformMatrix4fv(MVP_ID, 1, GL_FALSE, value_ptr(MVP));
        glUniformMatrix4fv(MV_ID, 1, GL_FALSE, value_ptr(MV));
        glUniformMatrix3fv(Normal_ID, 1, GL_FALSE, value_ptr(normalMatrix));

        GLuint start = 0;
        for (int j = 0; j < texInfo[i].size(); j++) {

          if (texInfo[i][j].z) {
              texUnit = texInfo[i][j].y;
          } else {
              texUnit = 0;
          }

          glProgramUniform1i(classic, isTextured, texInfo[i][j].z);
          glUniform1i(texLoc, texUnit);
          glDrawArrays( GL_TRIANGLES, start, texInfo[i][j].x);
          start += texInfo[i][j].x;
        }
    }

    glFlush();
} // display


GLfloat getScaleFactor() {
    GLfloat maxNum = max(max(maxVec.x, maxVec.y), maxVec.z);

    return maxNum/(maxVec.x + maxVec.y + maxVec.z);
}

void processSpecialKeys(int key, int x, int y) {
    glm::mat4 transformationMatrix(1.0f);
    glm::vec3 gaze = glm::normalize(center - eye);
    GLfloat scaleFactor = getScaleFactor();
    switch(key) {
        case GLUT_KEY_LEFT :
            transformationMatrix = glm::rotate(transformationMatrix, 3.14159f / 180.f, glm::vec3(0.0, 0.0, 1.0));
            eye = glm::vec3(transformationMatrix * glm::vec4(eye, 1.0));
            center = glm::vec3(transformationMatrix * glm::vec4(center, 1.0));
            break;
        case GLUT_KEY_RIGHT :
            transformationMatrix = glm::rotate(transformationMatrix, -3.14159f / 180.f, glm::vec3(0.0, 0.0, 1.0));
            eye = glm::vec3(transformationMatrix * glm::vec4(eye, 1.0));
            center = glm::vec3(transformationMatrix * glm::vec4(center, 1.0));
            break;
        case GLUT_KEY_UP :
            transformationMatrix = glm::translate(transformationMatrix, scaleFactor * gaze);
            eye = glm::vec3(transformationMatrix * glm::vec4(eye, 1.0));
            center = glm::vec3(transformationMatrix * glm::vec4(center, 1.0));
            break;
        case GLUT_KEY_DOWN :
            transformationMatrix = glm::translate(transformationMatrix, -scaleFactor * gaze);
            eye = glm::vec3(transformationMatrix * glm::vec4(eye, 1.0));
            center = glm::vec3(transformationMatrix * glm::vec4(center, 1.0));
            break;
    }

    viewMatrix=glm::lookAt( eye, center,viewUp);
    glutPostRedisplay();

} // processSpecialKeys

void processKeys ( unsigned char key, int x, int y )
{
    glm::mat4 transformationMatrix(1.0f);
    glm::vec3 gaze = glm::normalize(center - eye);
    GLfloat scaleFactor = getScaleFactor();
    switch (key ) {
        case 27: // escape key
        case 'q':
            exit( 0 );
            break;
        case 's': // set solid surface rasterization
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            break;
        case 'w':
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            break;
        case 'z' :
            transformationMatrix = glm::rotate(transformationMatrix,  -3.14159f / 180.f, gaze);
            viewUp = glm::vec3(transformationMatrix * glm::vec4(viewUp, 1.0));
            break;
        case 'x' :
            transformationMatrix = glm::rotate(transformationMatrix,  3.14159f / 180.f, gaze);
            viewUp = glm::vec3(transformationMatrix * glm::vec4(viewUp, 1.0));
            break;
        case 'c' :
            transformationMatrix = glm::translate(transformationMatrix, -scaleFactor * viewUp);
            eye = glm::vec3(transformationMatrix * glm::vec4(eye, 1.0));
            break;
        case 'v' :
            transformationMatrix = glm::translate(transformationMatrix, scaleFactor * viewUp);
            eye = glm::vec3(transformationMatrix * glm::vec4(eye, 1.0));
            break;
        case 'd' :
            transformationMatrix = glm::translate(transformationMatrix, -scaleFactor * viewUp);
            center = glm::vec3(transformationMatrix * glm::vec4(center, 1.0));
            break;
        case 'f' :
            transformationMatrix = glm::translate(transformationMatrix, scaleFactor * viewUp);
            center = glm::vec3(transformationMatrix * glm::vec4(center, 1.0));
            break;
        case 'r' :
            eye = defaultEye;
            center = defaultCenter;
            viewUp = glm::vec3(0.f,0.f,1.f);
            break;
    }

    viewMatrix=glm::lookAt( eye, center,viewUp);
    glutPostRedisplay();

} // processKeys

////////////////////////////////////////////////////////////////////////////
// reshape
//////////////////////////////////////////////////////////////////////////

void reshape(int w, int h) {

    glViewport(0,0, w, h);

    GLfloat aspect = (GLfloat)w/(GLfloat)h;

    projectionMatrix = glm::perspective(
        glm::radians(90.0f), // horizontal field of view
        aspect, // aspect ratio
        0.5f,   // near distance
        1000.0f
    );

    glutPostRedisplay();
} // reshape

////////////////////////////////////////////////////////////////////////
//	main
////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    program = (string) argv[0];

    if (argc - optind < 1)  {
        usage(cerr);
    } else {
        processOpts(argc, argv);
    }

    glutInit( &argc, argv );

    #ifdef __APPLE__
        glutInitDisplayMode( GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
    #else
        glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
        glutInitContextVersion( 4, 3 );
        glutInitContextProfile( GLUT_CORE_PROFILE );// GLUT_COMPATIBILITY_PROFILE );
    #endif

    glutInitWindowSize( 600, 600 );

    glutCreateWindow( argv[0] );
    glutReshapeFunc(reshape);

    glewExperimental = GL_TRUE;	// added for glew to work!
    if ( glewInit() )
    {
        cerr << "Unable to initialize GLEW ... exiting" << endl;
        exit (EXIT_FAILURE );
    }

    init();
    glutDisplayFunc( display );

    glutKeyboardFunc( processKeys );
    glutSpecialFunc( processSpecialKeys );
    glutMainLoop();

    return 0;
} // main

/************************************************************
 * processOpts
 *
 * Handles command line arguments
 *
 * @param   argc           Number of command line args
 * @param   *argv[]        command line args
 ************************************************************/

void processOpts(int argc,char *argv[]) {

    char ch; // placeholding character by getopt_long
    while ((ch = getopt(argc, argv, "uc:")) != -1)
        switch (ch) {
            case 'u': // usage
                usage(cout);
                break;
            case 'c' : // control file
                if (optarg != NULL){
                    controlFile = optarg;
                }
                break;
            case '?': // unknown argument
                usage(cerr);
                break;
            default: // Should not get here, but error if we do.
                cerr << "ERROR: Reached switch default in processOpts.";
                usage(cerr);
                break;

        }
} // processOpts

/************************************************************
 * usage
 *
 * Print usage string to a chosen output stream.
 *
 * @param   *progName       Name of the executable running
 * @param   *file           Output stream to print to
 ************************************************************/
void usage(std::ostream &file) {
    file << "Usage: " + program + " option [filename]\n"
            "     OPTIONS\n"
            "     -u,                print this information and exit\n"
            "     -c filename,       specify control file\n"
            "           Full path must be specified.\n";

    if ( file == cout ) // If printing to stdout, this is intentional.
        exit(0);          // Thus, exit normally.
    else
        exit(1);          // Otherwise, stop program with exit status 1.
} // usage

void commitCurrentObj(glm::mat4 modelMatrix,
                      vector<glm::vec4> &vertices,
                      vector<glm::vec3> &normals,
                      vector<glm::vec2> &textureVertices,
                      vector<glm::vec3> &ka,
                      vector<glm::vec3> &kd,
                      vector<glm::vec3> &ks,
                      vector<GLfloat> &ns,
                      vector<int> &textureMapOffset
                      ) {
    /////// as per assignment 1, gen and bind buffers
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    if (textureVertices.size() == 0) {
      for (int i = 0; i < vertices.size(); i++) {
        textureVertices.push_back(glm::vec2(0,0));
      }
    }

    GLuint vbo, nbo, abo, dbo, sbo, nsbo, tbo;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec4),
                    &vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &nbo);
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3),
                    &normals[0], GL_STATIC_DRAW);

    glGenBuffers(1, &abo);
    glBindBuffer(GL_ARRAY_BUFFER, abo);
    glBufferData(GL_ARRAY_BUFFER, ka.size() * sizeof(glm::vec3),
                    &ka[0], GL_STATIC_DRAW);


    glGenBuffers(1, &dbo);
    glBindBuffer(GL_ARRAY_BUFFER, dbo);
    glBufferData(GL_ARRAY_BUFFER, kd.size() * sizeof(glm::vec3),
                    &kd[0], GL_STATIC_DRAW);

    glGenBuffers(1, &sbo);
    glBindBuffer(GL_ARRAY_BUFFER, sbo);
    glBufferData(GL_ARRAY_BUFFER, ks.size() * sizeof(glm::vec3),
                    &ks[0], GL_STATIC_DRAW);


    glGenBuffers(1, &nsbo);
    glBindBuffer(GL_ARRAY_BUFFER, nsbo);
    glBufferData(GL_ARRAY_BUFFER, ns.size() * sizeof(GLfloat),
                    &ns[0], GL_STATIC_DRAW);

    glGenBuffers(1, &tbo);
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glBufferData(GL_ARRAY_BUFFER, textureVertices.size() * sizeof(glm::vec2),
                    &textureVertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(vPosition);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(
        vPosition, // position attribute
        4,          // size
        GL_FLOAT,   // type
        GL_FALSE,   // normalized?
        0,          // stride -- what?
        NULL
    );

    glEnableVertexAttribArray(vNormal);
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glVertexAttribPointer(
        vNormal, // position attribute
        3,          // size
        GL_FLOAT,   // type
        GL_FALSE,   // normalized?
        0,          // stride -- what?
        NULL
    );


    glEnableVertexAttribArray(vAmbient);
    glBindBuffer(GL_ARRAY_BUFFER, abo);
    glVertexAttribPointer(
        vAmbient, // position attribute
        3,          // size
        GL_FLOAT,   // type
        GL_FALSE,   // normalized?
        0,          // stride -- what?
        NULL
    );

    glEnableVertexAttribArray(vDiffuse);
    glBindBuffer(GL_ARRAY_BUFFER, dbo);
    glVertexAttribPointer(
        vDiffuse, // position attribute
        3,          // size
        GL_FLOAT,   // type
        GL_FALSE,   // normalized?
        0,          // stride -- what?
        NULL
    );

    glEnableVertexAttribArray(vSpecular);
    glBindBuffer(GL_ARRAY_BUFFER, sbo);
    glVertexAttribPointer(
        vSpecular, // position attribute
        3,          // size
        GL_FLOAT,   // type
        GL_FALSE,   // normalized?
        0,          // stride -- what?
        NULL
    );

    glEnableVertexAttribArray(vShiny);
    glBindBuffer(GL_ARRAY_BUFFER, nsbo);
    glVertexAttribPointer(
        vShiny, // position attribute
        1,          // size
        GL_FLOAT,   // type
        GL_FALSE,   // normalized?
        0,          // stride -- what?
        NULL
    );

    glEnableVertexAttribArray(vTexture);
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glVertexAttribPointer(
        vTexture, // position attribute
        2,          // size
        GL_FLOAT,   // type
        GL_FALSE,   // normalized?
        0,          // stride -- what?
        NULL
    );


    VAOS.push_back (vao);
    modelMatrices.push_back (modelMatrix);
    numVertices.push_back (vertices.size());

    ////// Locate texture changes /////
    vector<glm::vec3> temp;
    int vertCount = 0;
    int i;
    int isTex = 0;
    for (i = 0; i < vertices.size() - 1; i++) {
        ++vertCount;
        if (textureMapOffset[i] == -1) {
          isTex = 0;
        } else {
          isTex = 1;
        }
        if (textureMapOffset[i] != textureMapOffset[i+1]) {
            temp.push_back (glm::vec3(vertCount, textureMapOffset[i], isTex));
            vertCount = 0;
        }

    }
    temp.push_back (glm::vec3(vertCount + 1, textureMapOffset[i], isTex));
    texInfo.push_back(temp);


    ////// Find center of object /////////
    glm::vec3 sum = glm::vec3(0.f,0.f,0.f);
    for (int i = 0; i < vertices.size(); i++) {

        glm::vec3 temp = glm::vec3(modelMatrix * vertices[i]);
        sum += temp;

        if (temp.x > maxVec.x) {
            maxVec.x = temp.x;
        }

        if (temp.y > maxVec.y) {
            maxVec.y = temp.y;
        }

        if (temp.z > maxVec.z) {
            maxVec.z = temp.z;
        }
    }
    centers.push_back(
        glm::vec3(sum.x/vertices.size(),
        sum.y/vertices.size(),
        sum.z/vertices.size()));

    vertices.clear();
    normals.clear();
    textureVertices.clear();
    ka.clear();
    kd.clear();
    ks.clear();
    ns.clear();
    textureMapOffset.clear();

} // commitCurrentObj
