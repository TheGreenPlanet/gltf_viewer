#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
uniform vec3 u_lightColor;
uniform vec3 u_diffuseColor; // The diffuse surface color addition from ImGUI
uniform vec3 u_materialDiffuseColor; // The diffuse surface color of the model
uniform vec3 u_ambientColor;
uniform vec3 u_specularColor;
uniform float u_specularPower;
uniform samplerCube u_cubemap;
uniform sampler2D u_texture1;

uniform int u_diffuseEnabled;
uniform int u_specularEnabled;
uniform int u_lightEnabled;
uniform int u_ambientEnabled;
uniform bool u_showNormals;
uniform bool u_gammaCorrection;
uniform bool u_environmentMapping;
uniform bool u_showTexcoords;

// Fragment shader inputs
in vec3 L;      // View-space light vector
in vec3 N;      // View-space normal
in vec3 V;      // View vector (direction)
in vec3 v_normal;
in float v_distance;
in vec2 v_texcoord; // interpolated texture coordinate

// Fragment shader outputs
out vec4 frag_color;

void main()
{    
    
    // Calculate the diffuse (Lambertian) reflection term
    float diffuse = max(0.0, dot(N, L));
    
    // Ambient and specular (part 4)
    vec3 ambient = u_ambientColor;
    vec3 H = normalize(L + V);
    float specular = pow(dot(N,H), u_specularPower);
    
    //a_texture1.basecolor

    // Multiply the diffuse reflection term with the base surface color
    vec3 color = u_ambientEnabled * u_ambientColor + 
                u_diffuseEnabled * diffuse * (u_diffuseColor + u_materialDiffuseColor) * u_lightColor * u_lightEnabled / v_distance +
                (u_specularPower + 8.0) / 8.0 * specular * u_specularColor * u_lightColor * u_lightEnabled * u_specularEnabled / v_distance;
    if (u_showNormals) {
        color = 0.5 * v_normal + 0.5;
    }

    if (u_gammaCorrection) {
        color = pow(color, vec3(1 / 2.2));
    } 
    
    // Cube map
    if (u_environmentMapping) {
        vec3 R = reflect(-V, N);
        color = texture(u_cubemap, R).rgb;
    }

    // Visualize texture coordinates
    if (u_showTexcoords) {
        color = vec3(v_texcoord.x, v_texcoord.y,0);
        // color = texture(u_texture1, v_texcoord).rgb;
    }
    
    frag_color = vec4(color,1.0);
}
