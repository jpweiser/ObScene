// Vertex shader for multiple lights stays the same with all lighting
// done in the fragment shader.

#version 410 core

uniform mat4 MVPMatrix;	// full model-view-projection matrix
uniform mat4 MVMatrix;	// model and view matrix
uniform mat3 NormalMatrix;	// transformation matrix for normals to eye coordinates

//in vec4 VertexColor;   need to use material values instead; see fragment shader
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec3 ka;
layout(location = 3) in vec3 kd;
layout(location = 4) in vec3 ks;
layout(location = 5) in float ns;
layout(location = 6) in vec2 texIndex;

//out vec4 Color;
out vec3 Normal;	// vertex normal in eye coordinates
out vec4 Position;	// vertex position in eye coordinates

out vec3 ambient;
out vec3 diffuse;
out vec3 specular;
out float shininess;
out vec2 tIndex;

void main()
{
    tIndex = texIndex;

    ambient = ka;
    diffuse = kd;
    specular = ks;
    shininess = ns;

    Normal = normalize(NormalMatrix * VertexNormal);
    Position = MVMatrix * vec4(VertexPosition, 1.0f);
    gl_Position = MVPMatrix * vec4(VertexPosition, 1.0f);
}
