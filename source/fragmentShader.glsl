#version 330 core

# define maxLights 10

// Inputs
in vec2 UV;
in vec3 fragmentPosition;
in vec3 tangentSpaceLightPosition[maxLights];
in vec3 tangentSpaceLightDirection[maxLights];

// Outputs
out vec3 fragmentColour;

// Light struct
struct Light
{
    vec3 position;
    vec3 colour;
    vec3 direction;
    float constant;
    float linear;
    float quadratic;
    float cosPhi;
    int type;
};

// Uniforms
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform float ka;
uniform float kd;
uniform float ks;
uniform float Ns;
uniform Light lightSources[maxLights];

// Function prototypes
vec3 pointLight(vec3 lightPosition, vec3 lightColour,
                float constant, float linear, float quadratic);

vec3 spotLight(vec3 lightPosition, vec3 direction, vec3 lightColour,
               float cosPhi, float constant, float linear, float quadratic);

vec3 directionalLight(vec3 lightDirection, vec3 lightColour);

// Get the normal vector from the normal map
vec3 Normal = normalize(2.0 * vec3(texture(normalMap, UV)) - 1.0);

void main ()
{
    fragmentColour = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < maxLights; i++)
    {
        // Determine light properties for current light source
        vec3 lightPosition  = tangentSpaceLightPosition[i];
        vec3 lightColour    = lightSources[i].colour;
        vec3 lightDirection = tangentSpaceLightDirection[i];
        float constant      = lightSources[i].constant;
        float linear        = lightSources[i].linear;
        float quadratic     = lightSources[i].quadratic;
        float cosPhi        = lightSources[i].cosPhi;
        
        // Calculate point light
        if (lightSources[i].type == 1)
            fragmentColour += pointLight(lightPosition, lightColour,
                                         constant, linear, quadratic);
        
        // Calculate spotlight
        if (lightSources[i].type == 2)
            fragmentColour += spotLight(lightPosition, lightDirection, lightColour,
                                        cosPhi, constant, linear, quadratic);
           
        // Calculate directional light
        if (lightSources[i].type == 3)
            fragmentColour += directionalLight(lightDirection, lightColour);
    }
}

// Calculate point light
vec3 pointLight(vec3 lightPosition, vec3 lightColour,
                float constant, float linear, float quadratic)
{
    // Object colour
    vec3 objectColour = vec3(texture(diffuseMap, UV));
    
    // Ambient reflection
    vec3 ambient = ka * objectColour;
    
    // Diffuse reflection
    vec3 light      = normalize(lightPosition - fragmentPosition);
    vec3 normal     = normalize(Normal);
    float cosTheta  = max(dot(normal, light), 0);
    vec3 diffuse    = kd * lightColour * objectColour * cosTheta;
    
    // Specular reflection
    vec3 reflection = - light + 2 * dot(light, normal) * normal;
    vec3 camera     = normalize(-fragmentPosition);
    float cosAlpha  = max(dot(camera, reflection), 0);
    vec3 specular   = ks * lightColour * pow(cosAlpha, Ns);
    specular       *= vec3(texture(specularMap, UV));
    
    // Attenuation
    float distance    = length(lightPosition - fragmentPosition);
    float attenuation = 1.0 / (constant + linear * distance +
                               quadratic * distance * distance);
    
    // Fragment colour
    return (ambient + diffuse + specular) * attenuation;
}

// Calculate spotlight
vec3 spotLight(vec3 lightPosition, vec3 lightDirection, vec3 lightColour,
               float cosPhi, float constant, float linear, float quadratic)
{
    // Object colour
    vec3 objectColour = vec3(texture(diffuseMap, UV));
    
    // Ambient reflection
    vec3 ambient = ka * objectColour;
    
    // Diffuse reflection
    vec3 light     = normalize(lightPosition - fragmentPosition);
    vec3 normal    = normalize(Normal);
    float cosTheta = max(dot(normal, light), 0);
    vec3 diffuse   = kd * lightColour * objectColour * cosTheta;
    
    // Specular reflection
    vec3 reflection = - light + 2 * dot(light, normal) * normal;
    vec3 camera     = normalize(-fragmentPosition);
    float cosAlpha  = max(dot(camera, reflection), 0);
    vec3 specular   = ks * lightColour * pow(cosAlpha, Ns);
    specular       *= vec3(texture(specularMap, UV));
    
    // Attenuation
    float distance    = length(lightPosition - fragmentPosition);
    float attenuation = 1.0 / (constant + linear * distance +
                               quadratic * distance * distance);
    
    // Directional light intensity
    vec3 direction  = normalize(lightDirection);
    cosTheta        = dot(-light, direction);
    float delta     = radians(2.0);
    float intensity = clamp((cosTheta - cosPhi) / delta, 0.0, 1.0);
    
    // Return fragment colour
    return (ambient + diffuse + specular) * attenuation * intensity;
}

// Calculate directional light
vec3 directionalLight(vec3 lightDirection, vec3 lightColour)
{
    // Object colour
    vec3 objectColour = vec3(texture(diffuseMap, UV));
    
    // Ambient reflection
    vec3 ambient = ka * objectColour;
    
    // Diffuse reflection
    vec3 light     = normalize(-lightDirection);
    vec3 normal    = normalize(Normal);
    float cosTheta = max(dot(normal, light), 0);
    vec3 diffuse   = kd * lightColour * objectColour * cosTheta;
    
    // Specular reflection
    vec3 reflection = - light + 2 * dot(light, normal) * normal;
    vec3 camera     = normalize(-fragmentPosition);
    float cosAlpha  = max(dot(camera, reflection), 0);
    vec3 specular   = ks * lightColour * pow(cosAlpha, Ns);
    specular       *= vec3(texture(specularMap, UV));
    
    // Return fragment colour
    return ambient + diffuse + specular;
}
