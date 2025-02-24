#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
uniform float u_time;

// Uniform model matrices
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_model;

// Uniform world colors
uniform vec3 u_lightPosition; // The position of your light source


// Vertex inputs (attributes from vertex buffers)
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in vec2 a_texcoord;

// Vertex shader outputs 
out vec3 L; // View-space light vector
out vec3 N; // View-space normal
out vec3 V; // View vector (direction)
out vec3 v_normal;
out float v_distance;
out vec2 v_texcoord;

void main()
{
    // Part 2 (Why?): Nothing changed because we're multiplying with the identity matrices
    gl_Position = u_projection * u_view * u_model * a_position;

    mat4 mv = u_view * u_model;

    // Transform the vertex position to view space (eye coordinates)
    vec3 positionEye = vec3(mv * a_position);

    // Calculate the view-space normal
    N = normalize(mat3(mv) * a_normal);

    // Calculate the view-space light direction
    vec4 lightPositionView = mv * vec4(u_lightPosition, 1);
    L = normalize(vec3(lightPositionView) - positionEye);

    V = normalize(-positionEye);


    v_distance = dot(positionEye, positionEye) * 0.5;
    v_normal = a_normal;
    v_texcoord = a_texcoord;
}