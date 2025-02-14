#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
uniform float u_time;

// Uniform model matrices
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_model;

// Uniform world colors
uniform vec3 u_lightColor;
uniform vec3 u_diffuseColor; // The diffuse surface color of the model
uniform vec3 u_lightPosition; // The position of your light source
uniform vec3 u_ambientColor;
uniform vec3 u_specularColor;
uniform float u_specularPower;



uniform int u_diffuseEnabled;
uniform int u_specularEnabled;
uniform int u_lightEnabled;
uniform int u_ambientEnabled;
uniform bool u_showNormals;

// Vertex inputs (attributes from vertex buffers)
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec3 a_normal;

// Vertex shader outputs
// ...
out vec3 v_color;

void main()
{
    // Part 2 (Why?): Nothing changed because we're multiplying with the identity matrices
    gl_Position = u_projection * u_view * u_model * a_position;

    mat4 mv = u_view * u_model;

    // Transform the vertex position to view space (eye coordinates)
    vec3 positionEye = vec3(mv * a_position);

    // Calculate the view-space normal
    vec3 N = normalize(mat3(mv) * a_normal);

    // Calculate the view-space light direction
    vec3 L = normalize(u_lightPosition - positionEye);

    // Calculate the diffuse (Lambertian) reflection term
    float diffuse = max(0.0, dot(N, L));
    
    // Ambient and specular (part 4)
    vec3 ambient = u_ambientColor;
    vec3 viewDir = normalize(-positionEye);
    vec3 H = normalize(L + viewDir);
    float specular = pow(dot(N,H), u_specularPower);
    

    float distance = dot(positionEye, positionEye);

    // Multiply the diffuse reflection term with the base surface color
    v_color =   u_ambientEnabled * u_ambientColor + 
                u_diffuseEnabled * diffuse * u_diffuseColor * u_lightColor * u_lightEnabled / distance +
                u_specularEnabled * specular * u_specularColor * u_lightColor * u_lightEnabled / distance;
    if (u_showNormals) {
        v_color = 0.5 * a_normal + 0.5;
    }
}