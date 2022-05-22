#version 330 core

// Vertex position
layout(location = 0) in vec3 vertexPosition;

// Vertex color
layout(location = 1) in vec3 vertexColor;

// Vertex UV coordinate
layout(location = 2) in vec2 vertexUV;

// Vertex Normal
layout(location = 3) in vec3 vertexNormal;

// UV coordinate (will be passed to the fragment shader)
out vec2 outUV;

// Color (will be passed to the fragment shader)
out vec3 outColor;

//Vertex Normal
out vec3 fragNormal;

//
out vec3 FragPos;

//mvp matrix
uniform mat4 mvpMatrix, modelMatrix;

uniform mat4 mvpMatrixLight;

void main()
{
	gl_Position = mvpMatrixLight * vec4(vertexPosition, 1.0);
	outUV = vertexUV;
	outColor = vertexColor;
}