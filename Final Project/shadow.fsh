#version 330 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float farPlane;

// calculate depth manually
void main()
{
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // clamp to 0, 1
    lightDistance = lightDistance / farPlane;
    gl_FragDepth = lightDistance;
}  