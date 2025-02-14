#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
// ...

// Fragment shader inputs
// ...
in vec3 v_color;

// Fragment shader outputs
out vec4 frag_color;

void main()
{    
    frag_color = vec4(v_color, 1.0);
}
