#version 330 core

// UV-coordinate of the fragment (interpolated by the rasterization stage)
in vec2 outUV;

// Color of the fragment received from the vertex shader (interpolated by the rasterization stage)
in vec3 outColor;

// Final color of the fragment that will be rendered on the screen
out vec4 fragColor;

in vec3 FragPos;

in vec3 fragNormal;

// Texture unit of the texture
uniform sampler2D texture_diffuse1;

void main()
{
	// Get pixel color of the texture at the current UV coordinate
	// and output it as our final fragment color
	fragColor = texture(texture_diffuse1, outUV);
}