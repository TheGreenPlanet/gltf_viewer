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
uniform sampler2D u_bumpMap1;

uniform int u_diffuseEnabled;
uniform int u_specularEnabled;
uniform int u_lightEnabled;
uniform int u_ambientEnabled;
uniform bool u_showNormals;
uniform bool u_gammaCorrection;
uniform bool u_environmentMapping;
uniform bool u_showTexcoords;
uniform bool u_bumpMappingEnabled;
uniform bool u_hasBumpMap;
uniform bool u_showMaterial;
uniform bool u_hasTexture;

// Fragment shader inputs
in vec3 L;      // View-space light vector
in vec3 N;      // View-space normal
in vec3 V;      // View vector (direction)
in vec3 v_normal;
in float v_distance;
in vec2 v_texcoord; // interpolated texture coordinate

// Fragment shader outputs
out vec4 frag_color;


mat3 tangent_space(vec3 eye, vec2 texcoord, vec3 normal)
{
    // Compute pixel derivatives
    vec3 delta_pos1 = dFdx(eye);
    vec3 delta_pos2 = dFdy(eye);
    vec2 delta_uv1 = dFdx(texcoord);
    vec2 delta_uv2 = dFdy(texcoord);
    
    // Compute tangent space vectors
    vec3 N = normal;
    vec3 T = normalize(delta_pos1 * delta_uv2.y -
                        delta_pos2 * delta_uv1.y);
    vec3 B = normalize(delta_pos2 * delta_uv1.x -
                        delta_pos1 * delta_uv2.x);
    
    return mat3(T, B, N);
}

void main()
{    
    vec3 N2 = N;

    if (u_bumpMappingEnabled) {
        mat3 TBN = tangent_space(V, v_texcoord, N);
        vec3 normal_tangent = texture(u_bumpMap1, v_texcoord).rgb * 2.0 - 1.0;
        vec3 normal_world = normalize(TBN * normal_tangent);
        
        N2 = normal_world;
    }

    // Calculate the diffuse (Lambertian) reflection term
    float diffuse = max(0.0, dot(N2, L));
    
    // Ambient and specular (part 4)
    vec3 H = normalize(L + V);
    float specular = pow(dot(N2,H), u_specularPower);
    



    vec3 objectColor = vec3(1.0, 1.0, 1.0);
    if (u_showMaterial && u_hasTexture) {
        objectColor = texture(u_texture1, v_texcoord).rgb;
    }

    // Multiply the diffuse reflection term with the base surface color
    vec3 ambientPlusDiffuse = u_ambientEnabled * u_ambientColor + 
                u_diffuseEnabled * diffuse * (u_materialDiffuseColor + u_diffuseColor) * u_lightColor * 
                u_lightEnabled / v_distance;

    vec3 specularColor = (u_specularPower + 8.0) / 8.0 * specular * u_specularColor * u_lightColor * 
                u_lightEnabled * u_specularEnabled / v_distance;

    vec3 phongColor = ambientPlusDiffuse * objectColor + specularColor;


    if (u_showNormals) {
        phongColor = 0.5 * v_normal + 0.5;
    }

    if (u_gammaCorrection) {
        phongColor = pow(phongColor, vec3(1.0 / 2.2));
    } 
    
    // Cube map
    if (u_environmentMapping) {
        vec3 R = reflect(-V, N2);
        phongColor = texture(u_cubemap, R).rgb;
    }


    // Visualize texture coordinates
    if (u_showTexcoords) {
        phongColor = vec3(v_texcoord.x, v_texcoord.y,0);
        //color = texture(u_texture1, v_texcoord).rgb;
    }
    

    
    frag_color = vec4(phongColor, 1.0);
}
