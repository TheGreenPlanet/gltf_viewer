#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
uniform float u_time;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_model;

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
    v_color = 0.5 * a_normal + 0.5; // maps the normal direction to an RGB color
    //v_color = vec3(a_color.xy, a_color.z*2*sin(2*u_time));
}