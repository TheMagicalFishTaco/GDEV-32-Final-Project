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
uniform sampler2D texture_diffuse1, texture_specular1;
uniform PointLight pointLight;
uniform vec3 eyePos;
uniform samplerCube shadowMap;
uniform float farPlane;

vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);


float calculateShadow(vec3 normalVector, vec3 towardsLight)
{
	vec3 fragToLight = FragPos - vec3(0.0f, 0.0f, 0.0f);

	float depthValue = texture(shadowMap, fragToLight).r;
	float depthLightSpace = length(fragToLight);
	float bias = max(0.05f * (1.0f - dot(normalVector, towardsLight)), 0.005f);

	float newDepthValue = 0.0f;
	float shadowValue = 0.0f;
	vec2 shadowMapPixelSize = 1.0f / vec2(1024, 1024);
	float viewDistance = length(eyePos - FragPos);
    float diskRadius = (1.0 + (viewDistance / farPlane)) / 25.0f;
	for (int x = 0; x < 20; x++)
	{
		newDepthValue = texture(shadowMap, fragToLight + gridSamplingDisk[x] * diskRadius).r;
		newDepthValue *= farPlane; 
		if (newDepthValue < depthLightSpace - bias)
		{
			shadowValue += 0.0f;
		}
		else
		{
			shadowValue += 1.0f;
		}
	}

	return shadowValue;
}

void main()
{
	//ambient
	vec3 ambient = pointLight.ambient * texture(texture_diffuse1, outUV).rgb;

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
	
	vec3 PointComponent = (pointDiffuse + pointSpecular) * calculateShadow(norm, lightDir);
	
	// Get pixel color of the texture at the current UV coordinate
	// and output it as our final fragment color
	fragColor = vec4(ambient + PointComponent, 1.0);
}
