#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
uniform float u_time;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_model;
uniform vec3 u_diffuseColor; // The diffuse surface color of the model
uniform vec3 u_lightPosition; // The position of your light source

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

    // Multiply the diffuse reflection term with the base surface color
    v_color = diffuse * u_diffuseColor;
}