#version 330

// UV-coordinate of the fragment (interpolated by the rasterization stage)
in vec2 outUV;

// Color of the fragment received from the vertex shader (interpolated by the rasterization stage)
in vec3 outColor;

// Final color of the fragment that will be rendered on the screen
out vec4 fragColor;

in vec3 FragPos;

in vec3 fragNormal;

in vec4 lightFragPosition;

struct PointLight
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 position;
};

// Texture unit of the texture
uniform sampler2D texture_diffuse1, texture_specular1, shadowMap;
uniform PointLight pointLight;
uniform vec3 eyePos;

void main()
{
	//ambient
	vec3 ambient = pointLight.ambient * texture(texture_diffuse1, outUV).rgb;
	
	vec3 fragLightNDC = vec3(lightFragPosition.x / lightFragPosition.w, lightFragPosition.y / lightFragPosition.w, lightFragPosition.z / lightFragPosition.w);
	
	fragLightNDC.x = (fragLightNDC.x + 1) / 2;
	fragLightNDC.y = (fragLightNDC.y + 1) / 2;
	fragLightNDC.z = (fragLightNDC.z + 1) / 2;

	float sampledDepth = texture(shadowMap, fragLightNDC.xy).r;

	if (sampledDepth < fragLightNDC.z)
	{

	}
	else
	{
		
	}

	//diffuse
	vec3 norm = normalize(fragNormal);
	vec3 lightDir = normalize(pointLight.position - FragPos);
	float pointDist = length(pointLight.position - FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 pointDiffuse = diff * pointLight.diffuse * texture(texture_diffuse1, outUV).rgb;
	
	// specular
    vec3 viewDir = normalize(eyePos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 pointSpecular = spec * pointLight.specular * texture(texture_specular1,outUV).rgb * texture(texture_diffuse1, outUV).rgb;
	
	vec3 PointComponent = (pointDiffuse + pointSpecular);
	
	// Get pixel color of the texture at the current UV coordinate
	// and output it as our final fragment color
	fragColor = vec4(finalColor,1.0);
}
