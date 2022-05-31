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

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

float CalcLightAttenuation(float lightDistance)
{
    float lightConstant     = 1.0;
    float lightLinear       = 0.09;
    float lightQuadratic    = 0.032;

    float attenuation       = 1.0 / (lightConstant + (lightLinear * lightDistance) + (lightQuadratic * (lightDistance * lightDistance)));
    return clamp(attenuation, 0.0, 1.0);
}

float CalcSpotAttenuation(vec3 pointToLight, vec3 spotDirection, float outerConeCos, float innerConeCos)
{
    float spotDifference    = clamp(dot(spotDirection, -pointToLight), 0.0, 1.0);
    float attenuation       = (spotDifference - outerConeCos) / (innerConeCos - outerConeCos);
    return smoothstep(0.0, 1.0, attenuation);  
}

vec3 CalcBlinnPhongReflection(vec3 lightDir, vec3 lightColor, vec3 normal)
{
    float   shininess       = min(2048, max(0.001, (2.0 / pow(ubo.materialRoughness, 2))));

    vec3    viewPos         = ubo.cameraPosition;
    vec3    viewDir         = normalize(viewPos - fragPos);
    vec3    halfDir         = normalize(lightDir + viewDir);
    float   specAngle       = max(0.0, dot(halfDir, normal));
    float   specular        = pow(specAngle,  shininess);
    vec3    specularColor   = lightColor * ubo.materialSpecularColor * specular;
    return specularColor;
}

vec3 applyDirectionalLight(Light light, vec3 normal)
{
    float   lightDifference     = clamp(dot(normal, -light.direction), 0.0, 1.0);
    
    vec3    specularColor       = CalcBlinnPhongReflection(-light.direction, light.color, normal);
    return (light.color + specularColor) * lightDifference;
}

vec3 applyPointLight(Light light, vec3 normal, vec3 worldPos)
{
    vec3    lightToPixel        = light.position - worldPos;
    float   lightDistance       = length(lightToPixel);
    vec3    lightRay            = normalize(lightToPixel);
    
    float   lightDifference     = clamp(dot(normal, lightRay), 0.0, 1.0);
    float   lightAttenuation    = CalcLightAttenuation(lightDistance);

    vec3    specularColor       = CalcBlinnPhongReflection(lightRay, light.color, normal);
    return (light.color + specularColor) * lightAttenuation * lightDifference;
}

vec3 applySpotLight(Light light, vec3 normal, vec3 worldPos)
{
    vec3    lightToPixel        = light.position - worldPos;
    float   lightDistance       = length(lightToPixel);
    vec3    lightRay            = normalize(lightToPixel);
    
    float   lightDifference     = clamp(dot(normal, lightRay), 0.0, 1.0);
    float   lightAttenuation    = CalcLightAttenuation(lightDistance);

    float   spotAttenuation     = CalcSpotAttenuation(lightRay, light.direction, cos(light.outerAngle * 0.5), cos(light.innerAngle * 0.5));
    
    vec3    specularColor       = CalcBlinnPhongReflection(lightRay, light.color, normal);
    return (light.color + specularColor) * lightAttenuation * lightDifference * spotAttenuation;
}

vec3 applyAreaLight(Light light, vec3 normal, vec3 worldPos)
{
    vec3    lightToPixel        = light.position - worldPos;

    vec3    xVector             = normalize(cross(vec3(1.0, 0.0, 0.0) + light.direction, light.direction));
    vec3    yVector             = normalize(cross(xVector, light.direction));

    float   distanceToPlane     = dot(light.direction, -lightToPixel);
    vec3    pointOnPlane        = worldPos - (distanceToPlane * light.direction);

    vec3    lightToPoint        = pointOnPlane - light.position;
    
    vec2    area                = vec2(1.0, 1.0);
    vec2    nearest2D           = vec2(dot(lightToPoint, xVector), dot(lightToPoint, yVector));
            nearest2D           = vec2(clamp(nearest2D.x, -area.x, area.x), clamp(nearest2D.y, -area.y, area.y));
    vec3    closestPointInRect  = light.position + (xVector * nearest2D.x) + (yVector * nearest2D.y);

    vec3    pointToPixel        = closestPointInRect - worldPos;
    float   lightDistance       = length(pointToPixel);
    vec3    lightRay            = normalize(pointToPixel);
    
    float   lightDifference     = clamp(dot(normal, lightRay), 0.0, 1.0);
    float   lightAttenuation    = CalcLightAttenuation(lightDistance);

    float   spotAttenuation     = CalcSpotAttenuation(lightRay, light.direction, cos(light.outerAngle * 0.5), cos(light.innerAngle * 0.5));

    vec3    specularColor       = CalcBlinnPhongReflection(lightRay, light.color, normal);
    return (light.color + specularColor) * lightAttenuation * lightDifference * spotAttenuation;
}

void main()
{
    // diffuse
    vec4    diffuseColor    = texture(texSampler, fragTexCoord) * vec4(ubo.materialColor, 1.0f);
    
    // normal
    vec3    normal          = normalize(fragNormal);

    // ambient lighting
    vec3    ambientColor    = ubo.ambientLightColor * ubo.ambientLightIntensity;

    // lighting
    vec3    lightColor      = vec3(0.0, 0.0, 0.0);

    // sun light
    Light sun;
    sun.type        = LightType_Directional;
    sun.direction   = ubo.directionalLightDirection;
    sun.color       = ubo.directionalLightColor * ubo.directionalLightIntensity;

    lightColor += applyDirectionalLight(sun, normal);

    // dynamic lights
    for (int i = 0; i < ubo.lightCount; ++i)
    {
        Light light = ubo.lights[i];
        if (light.type == LightType_Directional)
        {
            lightColor += applyDirectionalLight(light, normal);
        }
        else if (light.type == LightType_Point)
        {
            lightColor += applyPointLight(light, normal, fragPos);
        }
        else if (light.type == LightType_Spot)
        {
            lightColor += applySpotLight(light, normal, fragPos);
        }
        else if (light.type == LightType_Area)
        {
            lightColor += applyAreaLight(light, normal, fragPos);
        }
    }

    // set fragment color
    outColor.xyz            = (lightColor + ambientColor) * diffuseColor.xyz;
    outColor.a              = diffuseColor.a;

    // Debug Normals
    //vec3 encodedNormal = (vec3(1.0, 1.0, 1.0) + normal) * 0.5;
    //outColor = vec4(encodedNormal, 1.0);
}