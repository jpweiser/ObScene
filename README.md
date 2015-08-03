# ObScene
Render WaveFront OBJ files using classic lighting model

Overview:

This program renders simple triangulated .obj files into a scene.
The files are parsed based on instructions from a control file,
which must be specified at the terminal when executing the program. For .mtl
files associated with some .obj files, it is assumed by the program that both
files are in the same directory. Correct relative filepaths to control files
must be provided, and similarly with .obj files. An example of this structure
should accompany the program.

--------------------------------------------------------------------------------
================================================================================
Build Instructions:

Use the included Makefile. At the terminal:

    $ make

Special instructions for Mac users:

To make sure the correct libraries are linked, use the following:

    $ make mac
--------------------------------------------------------------------------------
================================================================================
Running the Program:

./obscene option [filename]
     OPTIONS
     -u,                print this information and exit
     -c filename,       specify control file
           Full path must be specified.

--------------------------------------------------------------------------------
================================================================================
Example:

The following shows an actual use of included control file.

    $ ./obscene -c ExampleScene/voyager.txt

Use each in the fashion
    $ ./obscene -c controlFiles/[control file]

--------------------------------------------------------------------------------
================================================================================
Controlling the Program:


ESC or q:...........exit program
r:..................reset view to default values
w:..................render objects as wireframe
s:..................render objects as solid

← (left arrow):.....1 degree clockwise rotation around the eye position
                    and focal point around the z axis
→ (right arrow):....1 degree counter-clockwise rotation around the eye position
                    and focal point around the z axis
↑ (up arrow):.......Move camera forward along gaze vector
↓(down arrow):......Move camera back along gaze vector

c:..................move the camera down along view up vector
v:..................move camera up along view up vector

d:..................move the focal point in opposite direction of view up vector
f:..................move focal point in direction of view up vector

z:..................rotate view up vector by 1 degree counter-clockwize around
                    gaze vector
x:..................rotate view up vector by 1 degree clockwise around gaze
                    vector

--------------------------------------------------------------------------------
================================================================================
Some Default Values Used:

In absence of a specified or missing .mtl file,
KA = vec3(0.1,0.1,0.1)
KD = vec3(0.9,0.9,0.9)

--------------------------------------------------------------------------------
================================================================================
References:

OpenGL Programming (wiki-book)
http://en.wikibooks.org/wiki/OpenGL_Programming

OpenGL-Tutorial
http://www.opengl-tutorial.org/

Lighthouse3D
http://www.lighthouse3d.com/tutorials/

GLM documentation:
http://glm.g-truc.net/0.9.4/api/index.html

OpenGL documentation:
https://www.opengl.org/archives/resources/faq/technical/clipping.htm

paulburke.net
http://paulbourke.net/dataformats/

cplusplus.com -- absolutely crucial for learning c++
http://www.cplusplus.com/reference/

StackOverflow -- post on parsing stringstreams with delimited string
http://stackoverflow.com/questions/1894886/parsing-a-comma-delimited-stdstring
