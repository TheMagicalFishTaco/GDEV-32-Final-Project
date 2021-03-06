#version 330

// Vertex position
layout(location = 0) in vec3 vertexPosition;

uniform mat4 modelMatrix;

void main()
{
    gl_Position = modelMatrix * vec4(vertexPosition, 1.0f);
}