// Model viewer code for the assignments in Computer Graphics 1TD388/1MD150.
//
// Modify this and other source files according to the tasks in the instructions.
//

#include "gltf_io.h"
#include "gltf_scene.h"
#include "gltf_render.h"
#include "cg_utils.h"
#include "cg_trackball.h"

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdlib>
#include <iostream>
#include <cmath>

#include <array>


constexpr uint32_t CUBEMAP_MAX_DIRS = 5;
constexpr uint32_t CUBEMAP_PREFILTERED_MAX_NUMBER = 8;
// Struct for our application context
struct Context {
    int width = 1920;
    int height = 1080;
    GLFWwindow *window;
    gltf::GLTFAsset asset;
    gltf::DrawableList drawables;
    cg::Trackball trackball;
    GLuint program;
    GLuint emptyVAO;
    GLuint texture;
    float elapsedTime;
    std::string gltfFilename = "lpshead.gltf";
    glm::vec3 backgroundColor = glm::vec3(0.0f, 0.0f, 0.0f);
    // Add more variables here...

    // These are node specific variables, but we just put them here for now
    glm::vec3 diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 specularColor = glm::vec3(0.5f,0.5f,0.5f);
    float specularPower = 1.0f;
    glm::vec3 ambientColor = glm::vec3(0.5f,0.5f,0.5f);
    glm::vec3 lightColor = glm::vec3(0.5f,0.5f,0.5f);

    glm::vec3 lightPosition = glm::vec3(1.0f, 1.0f, 1.0f);
    float zoom_factor = 0.5f;

    bool diffuseEnabled = true;
    bool specularEnabled = true;
    bool lightEnabled = true;
    bool ambientEnabled = true;
    bool showNormals = false;
    bool showOrtho = false;
    bool gammeCorrection = true;
    bool environmentMapping = false;
    bool showTexcoords = false;

    // std::vector<string> textureIDs = {}; std::vector([])
    uint32_t activeCubemapLevel = 0;
    uint32_t cubemapTextureDir = 0;
    std::array<std::array<GLuint, CUBEMAP_PREFILTERED_MAX_NUMBER>, CUBEMAP_MAX_DIRS> cubemapTextures = {0};
    // std::vector<std::string> cubemapDirs = {"debug", "Forrest", "LarnacaCastle", "reference", "RomeChurch"};
    //const char* cubemapDirs = "debug\0Forrest\0LarnaCastle\0reference\0RomeChurch\0";
    const char *cubemapDirs[CUBEMAP_MAX_DIRS] = {"debug", "Forrest", "LarnacaCastle", "reference", "RomeChurch"};
    const char *roughnessLevels[CUBEMAP_PREFILTERED_MAX_NUMBER] = {"2048", "512", "128", "32", "8", "2", "0.5", "0.125"};
    gltf::TextureList textures;
};

/*
GLint cubemap_romechurch = cg::load_cubemap(cubemap_dir() + "/RomeChurch/");
GLint cubemap_forrest = cg::load_cubemap(cubemap_dir() + "/Forrest/");

ctx.cubemap_romechurch = ...
ctx.loaded_textures.push_back(cubemap_romechurch);


*/


// Returns the absolute path to the cubemap directory
std::string cubemap_dir()
{
    std::string rootDir = cg::get_env_var("MODEL_VIEWER_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: MODEL_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/assets/cubemaps/";
}

// Returns the absolute path to the src/shader directory
std::string shader_dir(void)
{
    std::string rootDir = cg::get_env_var("MODEL_VIEWER_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: MODEL_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/src/shaders/";
}

// Returns the absolute path to the assets/§tf directory
std::string gltf_dir(void)
{
    std::string rootDir = cg::get_env_var("MODEL_VIEWER_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: MODEL_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/assets/gltf/";
}

void store_cubemaps(Context &ctx) {

    

    for (uint32_t i = 0; i < CUBEMAP_MAX_DIRS; i++) {
        for (uint32_t j = 0; j < CUBEMAP_PREFILTERED_MAX_NUMBER; j++) {
            std::cout << "Loading cubemap: " << cubemap_dir() + ctx.cubemapDirs[i] + "/prefiltered/" + ctx.roughnessLevels[j] + "/" << std::endl;
            ctx.cubemapTextures[i][j] = cg::load_cubemap(cubemap_dir() + ctx.cubemapDirs[i] + "/prefiltered/" + ctx.roughnessLevels[j] + "/");
        }
    }
}

void do_initialization(Context &ctx)
{
    ctx.program = cg::load_shader_program(shader_dir() + "mesh.vert", shader_dir() + "mesh.frag");
    store_cubemaps(ctx);
    // ctx.texture = cg::load_cubemap(cubemap_dir() + "/RomeChurch/");

    gltf::load_gltf_asset(ctx.gltfFilename, gltf_dir(), ctx.asset);
    gltf::create_drawables_from_gltf_asset(ctx.drawables, ctx.asset);
    gltf::create_textures_from_gltf_asset(ctx.textures, ctx.asset);
}

void draw_scene(Context &ctx)
{
    // Activate shader program
    glUseProgram(ctx.program);

    // Set render state
    glEnable(GL_DEPTH_TEST);  // Enable Z-buffering
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP,ctx.cubemapTextures[ctx.cubemapTextureDir][ctx.activeCubemapLevel]);
    glUniform1i(glGetUniformLocation(ctx.program, "u_cubemap"), 0);
    

    // Define per-scene uniforms
    glUniform1f(glGetUniformLocation(ctx.program, "u_time"), ctx.elapsedTime);
    
    // ...
    glUniform1i(glGetUniformLocation(ctx.program, "u_diffuseEnabled"), ctx.diffuseEnabled);
    glUniform1i(glGetUniformLocation(ctx.program, "u_specularEnabled"), ctx.specularEnabled);
    glUniform1i(glGetUniformLocation(ctx.program, "u_lightEnabled"), ctx.lightEnabled);
    glUniform1i(glGetUniformLocation(ctx.program, "u_ambientEnabled"), ctx.ambientEnabled);
    glUniform1i(glGetUniformLocation(ctx.program, "u_showNormals"), ctx.showNormals);
    glUniform1i(glGetUniformLocation(ctx.program, "u_gammaCorrection"), ctx.gammeCorrection);
    glUniform1i(glGetUniformLocation(ctx.program, "u_environmentMapping"), ctx.environmentMapping);
    glUniform1i(glGetUniformLocation(ctx.program, "u_showTexcoords"), ctx.showTexcoords);

    float aspect = (float)ctx.width / (float)ctx.height;
    glm::mat4 view = glm::mat4(ctx.trackball.orient);
    view = view * glm::lookAt(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0,0,1));
    glm::mat4 projection = ctx.showOrtho
        ? glm::ortho(-1.0f *aspect, 1.0f * aspect, -1.0f, 1.0f, -10.0f, 10.0f) // Orthographic
        : glm::perspective(glm::radians(65.0f*ctx.zoom_factor), aspect, 0.1f, 100.0f); // Perspective



    // Draw scene
    for (unsigned i = 0; i < ctx.asset.nodes.size(); ++i) {
        const gltf::Node &node = ctx.asset.nodes[i];
        const gltf::Drawable &drawable = ctx.drawables[node.mesh];

        // Define per-object uniforms
        glm::mat4 model = glm::mat4(1.0f);

        // Apply model transformation
        model = glm::scale(model, glm::vec3(node.scale));
        model = glm::translate(model, node.translation);
        model = glm::rotate(model, node.rotationX, glm::vec3(1,0,0));
        model = glm::rotate(model, node.rotationY, glm::vec3(0,1,0));
        model = glm::rotate(model, node.rotationZ, glm::vec3(0,0,1));
            
        // Draw object
        glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_model"), 1, GL_FALSE, &model[0][0]);

        glUniform3fv(glGetUniformLocation(ctx.program, "u_diffuseColor"), 1, &ctx.diffuseColor[0]);
        glUniform3fv(glGetUniformLocation(ctx.program, "u_lightPosition"), 1, &ctx.lightPosition[0]);
        
        glUniform3fv(glGetUniformLocation(ctx.program, "u_ambientColor"), 1, &ctx.ambientColor[0]);
        glUniform3fv(glGetUniformLocation(ctx.program, "u_specularColor"), 1, &ctx.specularColor[0]);
        glUniform3fv(glGetUniformLocation(ctx.program, "u_lightColor"), 1, &ctx.lightColor[0]);
        glUniform1f(glGetUniformLocation(ctx.program, "u_specularPower"), ctx.specularPower);

        // Assignment 3 part 3, material textures.
        const gltf::Mesh &mesh = ctx.asset.meshes[node.mesh];
        if (mesh.primitives[0].hasMaterial) {
            const gltf::Primitive &primitive = mesh.primitives[0];
            const gltf::Material &material = ctx.asset.materials[primitive.material];
            const gltf::PBRMetallicRoughness &pbr = material.pbrMetallicRoughness;

            // Define material textures and uniforms
            if (pbr.hasBaseColorTexture) {
                GLuint texture_id = ctx.textures[pbr.baseColorTexture.index];
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, texture_id);
                glUniform1i(glGetUniformLocation(ctx.program, "u_texture1"), 1);
                glUniform3fv(glGetUniformLocation(ctx.program, "u_materialDiffuseColor"), 1, &pbr.baseColorFactor[0]);
                // Bind texture and define uniforms...
            } else {
                // Need to handle this case as well, by telling
                // the shader that no texture is available
            }
        }                

        glBindVertexArray(drawable.vao);
        glDrawElements(GL_TRIANGLES, drawable.indexCount, drawable.indexType,
                       (GLvoid *)(intptr_t)drawable.indexByteOffset);
        glBindVertexArray(0);
    }

    // Clean up
    cg::reset_gl_render_state();
    glUseProgram(0);
}

void do_rendering(Context &ctx)
{
    // Clear render states at the start of each frame
    cg::reset_gl_render_state();

    // Clear color and depth buffers
    
    glClearColor(ctx.backgroundColor.r, ctx.backgroundColor.g, ctx.backgroundColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_scene(ctx);
}

void reload_shaders(Context *ctx)
{
    glDeleteProgram(ctx->program);
    ctx->program = cg::load_shader_program(shader_dir() + "mesh.vert", shader_dir() + "mesh.frag");
}

void error_callback(int /*error*/, const char *description)
{
    std::cerr << description << std::endl;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_R && action == GLFW_PRESS) { reload_shaders(ctx); }
}

void char_callback(GLFWwindow *window, unsigned int codepoint)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_CharCallback(window, codepoint);
    if (ImGui::GetIO().WantTextInput) return;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse) return;

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        ctx->trackball.center = glm::vec2(x, y);
        ctx->trackball.tracking = (action == GLFW_PRESS);
    }
}

void cursor_pos_callback(GLFWwindow *window, double x, double y)
{
    // Forward event to ImGui
    if (ImGui::GetIO().WantCaptureMouse) return;

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    cg::trackball_move(ctx->trackball, float(x), float(y));
}

void scroll_callback(GLFWwindow *window, double x, double y)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_ScrollCallback(window, x, y);

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    ctx->zoom_factor -= y * 0.1f;

    if (ImGui::GetIO().WantCaptureMouse) return;
}

void resize_callback(GLFWwindow *window, int width, int height)
{
    // Update window size and viewport rectangle
    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    ctx->width = width;
    ctx->height = height;
    glViewport(0, 0, width, height);
}

int main(int argc, char *argv[])
{
    Context ctx = Context();
    if (argc > 1) { ctx.gltfFilename = std::string(argv[1]); }

    // Create a GLFW window
    glfwSetErrorCallback(error_callback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    ctx.window = glfwCreateWindow(ctx.width, ctx.height, "Model viewer", nullptr, nullptr);
    glfwMakeContextCurrent(ctx.window);
    glfwSetWindowUserPointer(ctx.window, &ctx);
    glfwSetKeyCallback(ctx.window, key_callback);
    glfwSetCharCallback(ctx.window, char_callback);
    glfwSetMouseButtonCallback(ctx.window, mouse_button_callback);
    glfwSetCursorPosCallback(ctx.window, cursor_pos_callback);
    glfwSetScrollCallback(ctx.window, scroll_callback);
    glfwSetFramebufferSizeCallback(ctx.window, resize_callback);

    // Load OpenGL functions
    if (gl3wInit() || !gl3wIsSupported(3, 3) /*check OpenGL version*/) {
        std::cerr << "Error: failed to initialize OpenGL" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    // Initialize ImGui
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(ctx.window, false /*do not install callbacks*/);
    ImGui_ImplOpenGL3_Init("#version 330" /*GLSL version*/);
    
    // Initialize rendering
    glGenVertexArrays(1, &ctx.emptyVAO);
    glBindVertexArray(ctx.emptyVAO);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    do_initialization(ctx);

    // Start rendering loop
    while (!glfwWindowShouldClose(ctx.window)) {
        glfwPollEvents();
        ctx.elapsedTime = glfwGetTime();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // ImGui::ShowDemoWindow();

        ImGui::Begin("Model viewer");

        ImGui::Text("Model");
        for (auto &node : ctx.asset.nodes) {
            ImGui::Text("%s", node.name.c_str());
            ImGui::SliderFloat3("Scale", &node.scale[0], 0.0f, 2.0f);
            ImGui::SliderFloat3("Translation", &node.translation[0], -10.0f, 10.0f);
            ImGui::SliderFloat("Rotation X", &node.rotationX, -6.28f, 6.28f);
            ImGui::SliderFloat("Rotation Y", &node.rotationY, -6.28f, 6.28f);
            ImGui::SliderFloat("Rotation Z", &node.rotationZ, -6.28f, 6.28f);
            ImGui::Spacing();
        }

        ImGui::ColorEdit3("Background color", &ctx.backgroundColor[0]);

        //ImGui::Text("Model transformation");
        //ImGui::SliderFloat("Scale", &ctx.modelScaleValue, 0.0f, 2.0f);
        //ImGui::SliderFloat3("Translation", &ctx.modelTranslationValue[0], -10.0f, 10.0f);
        //ImGui::SliderFloat("Rotation Angle", &ctx.modelAngleValue, -3.14f, 3.14f);
        //ImGui::SliderFloat3("Rotation Axis", &ctx.modelAxisValue[0], -1.0f, 1.0f);
        ImGui::Text("Material");
        ImGui::ColorEdit3("Diffuse color", &ctx.diffuseColor[0]);
        ImGui::Checkbox("Diffuse enabled", &ctx.diffuseEnabled);
        ImGui::ColorEdit3("Specular color", &ctx.specularColor[0]);
        ImGui::SliderFloat("Specular power", &ctx.specularPower, 0.0f, 100.0f);
        ImGui::Checkbox("Specular enabled", &ctx.specularEnabled);

        ImGui::Text("Lighting");
        ImGui::ColorEdit3("Light color", &ctx.lightColor[0]);
        ImGui::SliderFloat3("Light position", &ctx.lightPosition[0], -10.0f, 10.0f);
        ImGui::Checkbox("Light enabled", &ctx.lightEnabled);
        ImGui::ColorEdit3("Ambient color", &ctx.ambientColor[0]);
        ImGui::Checkbox("Ambient enabled", &ctx.ambientEnabled);

        ImGui::Text("Misc");
        ImGui::Checkbox("Show normals", &ctx.showNormals);
        ImGui::Checkbox("Show ortho", &ctx.showOrtho);
        ImGui::Checkbox("Gamma correction", &ctx.gammeCorrection);
        ImGui::Checkbox("Environment mapping", &ctx.environmentMapping);
        ImGui::Checkbox("Visualize Texture Coordinates", &ctx.showTexcoords);
        ImGui::Text("Fovy: %f %f", 65.0f*ctx.zoom_factor, glm::radians(65.0f*ctx.zoom_factor));

        ImGui::Text("Cubemap");
        ImGui::Combo("Cubemap", (int*)&ctx.cubemapTextureDir, ctx.cubemapDirs, CUBEMAP_MAX_DIRS);
        ImGui::Combo("Cubemap Roughness", (int*)&ctx.activeCubemapLevel, ctx.roughnessLevels, CUBEMAP_PREFILTERED_MAX_NUMBER);


        // .heap -> text2 -> "asdfasdf"

        // .data section
        // 0x1338 -> "asdfasdf"
        

        ImGui::End();

        do_rendering(ctx);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(ctx.window);
    }

    // Shutdown
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(ctx.window);
    glfwTerminate();
    std::exit(EXIT_SUCCESS);
}
