#version 450
#extension GL_ARB_separate_shader_objects : enable

const int LightType_Directional = 0;
const int LightType_Point = 1;
const int LightType_Spot = 2;
const int LightType_Area = 3;

struct Light
{
    vec3  position;
    int   type;

    vec3  direction;
    float range;
          
    vec3  color;
    float intensity;
          
    float innerAngle;
    float outerAngle;
};

layout(std140, binding = 0) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
    
    float ambientLightIntensity;
    vec3  ambientLightColor;
    
    float directionalLightIntensity;
    vec3  directionalLightColor;
    vec3  directionalLightDirection;

    vec3  materialColor;
    vec3  materialSpecularColor;
    float materialRoughness;

    int   lightCount;
    Light lights[8];
} ubo;

layout(std140, push_constant) uniform UniformPushConstant 
{
    mat4 model;
} upc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragNormal;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = ubo.proj * ubo.view * upc.model * vec4(inPosition, 1.0);

    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragNormal = mat3(transpose(inverse(upc.model))) * inNormal;
    fragPos = (upc.model * vec4(inPosition, 1.0)).xyz;
}
