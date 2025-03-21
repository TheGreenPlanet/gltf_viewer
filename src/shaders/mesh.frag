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

uniform sampler2D u_shadowMap; // the depth texture (shadowmap)
uniform mat4 u_shadowFromView; // transforms main view-space -> shadow clip-space



uniform bool u_diffuseEnabled;
uniform bool u_specularEnabled;
uniform bool u_lightEnabled;
uniform int u_ambientEnabled;
uniform float u_shadowBias;
uniform bool u_showNormals;
uniform bool u_gammaCorrection;
uniform bool u_environmentMapping;
uniform bool u_showTexcoords;
uniform bool u_bumpMappingEnabled;
uniform bool u_hasBumpMap;
uniform bool u_showMaterial;
uniform bool u_hasTexture;
uniform bool u_enableShadowmap;

// Fragment shader inputs
in vec3 L;      // View-space light vector
in vec3 N;      // View-space normal
in vec3 V;      // View vector (direction)
in vec3 v_normal;
in float v_distance;
in vec2 v_texcoord; // interpolated texture coordinate

in vec3 fragPosLight;

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
    vec3 N = normalize(normal);
    vec3 T = normalize(delta_pos1 * delta_uv2.y -
                        delta_pos2 * delta_uv1.y);
    vec3 B = normalize(delta_pos2 * delta_uv1.x -
                        delta_pos1 * delta_uv2.x);
    
    return mat3(T, B, N);
}

float shadowmap_visibility(sampler2D shadowmap, vec4 shadowPos, float bias)
{
    vec2 delta = vec2(0.5) / textureSize(shadowmap, 0).xy;
    vec2 texcoord = (shadowPos.xy / shadowPos.w) * 0.5 + 0.5;
    float depth = (shadowPos.z / shadowPos.w) * 0.5 + 0.5;
    
    // Sample the shadowmap and compare texels with (depth - bias) to
    // return a visibility value in range [0, 1]. If you take more
    // samples (using delta to offset the texture coordinate), the
    // returned value should be the average of all comparisons.
    float texel = texture(shadowmap, texcoord).r;
    float visibility = float(texel > (depth - bias));
    return visibility;
}

void main()
{    
    vec3 N2 = N;

    if (u_bumpMappingEnabled && u_hasBumpMap) {
        mat3 TBN = tangent_space(V, v_texcoord, N);
        // === FOR RGB NORMAL MAPS === //
        //normal_tangent = texture(u_bumpMap1, v_texcoord).r * 2.0 - 1.0; 
        // vec3 normal_world = TBN * vec3(normal_tangent;
        //N2 = normal_world;

        // === FOR GRAY BUMP MAPS === //
        // Calculating new normals using gray-scale bump map and surrounding values (Couldn't find examples of this, so I asked ChatGPT since it was optional anyway)

        // Sample height values (grayscale bump map)
        float heightCenter = texture(u_bumpMap1, v_texcoord).r; 
        float heightRight  = texture(u_bumpMap1, v_texcoord + vec2(0.001, 0.0)).r; 
        float heightUp     = texture(u_bumpMap1, v_texcoord + vec2(0.0, 0.001)).r;

        // Compute the slope of the height map (finite differences)
        vec3 normal_tangent;
        float strength = 4.0f; // Change the strength of the bump map. 
        normal_tangent.x = (heightCenter - heightRight) * strength;  // Partial derivative in U direction
        normal_tangent.y = (heightCenter - heightUp) * strength;     // Partial derivative in V direction
        normal_tangent.z = 1.0;                         // Default Z component to keep it unit-length

        // Normalize the normal to ensure proper shading
        normal_tangent = normalize(normal_tangent);
        
        vec3 normal_world = TBN * normal_tangent;
        N2 = normalize(normal_world);
    }

    // Calculate the diffuse (Lambertian) reflection term
    float diffuse = max(0.0, dot(N2, L));
    
    // Ambient and specular (part 4)
    vec3 H = normalize(L + V);
    float specular = pow(max(dot(N2,H),0.0f), u_specularPower);


    // Multiply the diffuse reflection term with the base surface color
    vec3 ambientColor = u_ambientEnabled * u_ambientColor;
    
    vec3 diffuseColor = (u_diffuseEnabled && u_lightEnabled)
        ? diffuse * u_diffuseColor * u_lightColor / v_distance
        : vec3(0.0f);
    

    vec3 objectColor = vec3(1.0, 1.0, 1.0);
    if (u_showMaterial && u_hasTexture) {
        objectColor = texture(u_texture1, v_texcoord).rgb;
        diffuseColor *= u_materialDiffuseColor; 
    }

    vec3 specularColor = (u_specularEnabled && u_lightEnabled) 
        ? ((u_specularPower + 8.0) / 8.0) * specular * u_specularColor * u_lightColor / v_distance
        : vec3(0.0f);

    // Shadow mapping
    if (u_enableShadowmap) {
        float visibility = shadowmap_visibility(u_shadowMap, u_shadowFromView * vec4(-V, 1.0f), u_shadowBias);
        diffuseColor *= visibility;
        specularColor *= visibility;
    }

    vec3 ambientPlusDiffuse = ambientColor + diffuseColor;
    vec3 phongColor = ambientPlusDiffuse * objectColor + specularColor;

    if (u_showNormals) {
        phongColor = 0.5 * v_normal + 0.5; //changed to N2 from v_normal
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
    }

    frag_color = vec4(phongColor, 1.0);
}
