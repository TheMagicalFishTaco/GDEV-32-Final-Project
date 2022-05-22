#version 330

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

uniform vec3 lightColor, lightPos, eyePos;

void main()
{
	//ambient
	float ambientStrength = 0.1f;
	vec3 ambient = ambientStrength * lightColor;
	
	//diffuse
	vec3 norm = normalize(fragNormal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * lightColor;
	
	// specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(eyePos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
	
	vec3 finalColor = (ambient + diffuse + specular);
	
	// Get pixel color of the texture at the current UV coordinate
	// and output it as our final fragment color
	fragColor = texture(texture_diffuse1, outUV) * vec4(finalColor,1.0);
}
