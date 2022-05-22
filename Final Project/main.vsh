#version 330

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



void main()
{
	// Convert our vertex position to homogeneous coordinates by introducing the w-component.
	// Vertex positions are ... positions, so we specify the w-coordinate as 1.0.
	vec4 finalPosition = vec4(vertexPosition, 1.0);
	
	FragPos = vec3(modelMatrix * finalPosition);
    fragNormal = mat3(transpose(inverse(modelMatrix))) * vertexNormal;  
	
	finalPosition = mvpMatrix * finalPosition;

	// Give OpenGL the final position of our vertex
	gl_Position = finalPosition;

	outUV = vertexUV;
	outColor = vertexColor;
}
