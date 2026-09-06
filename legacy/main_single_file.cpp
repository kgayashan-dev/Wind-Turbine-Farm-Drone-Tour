// OpenGL drawing.
#include <OpenGL/gl3.h>
// OpenGL is included separately.
#define GLFW_INCLUDE_NONE
// Window and input.
#include <GLFW/glfw3.h>

// Control panel widgets.
#include <imgui.h>
// UI input.
#include <imgui_impl_glfw.h>
// UI drawing.
#include <imgui_impl_opengl3.h>

// Vectors and matrices.
#include <glm/glm.hpp>
// Move, rotate, and scale.
#include <glm/gtc/matrix_transform.hpp>
// Send maths data to OpenGL.
#include <glm/gtc/type_ptr.hpp>

// Limit values.
#include <algorithm>
// Maths functions.
#include <cmath>
// Exit codes.
#include <cstdlib>
// Console messages.
#include <iostream>
// Text storage.
#include <string>
// Lists of points and triangles.
#include <vector>

namespace
{

    constexpr float PI = 3.14159265358979323846f;

    // Stores the GPU buffer IDs and triangle count of one 3D shape.

    struct Mesh
    {
        GLuint vao = 0, vbo = 0, ebo = 0;
        GLsizei count = 0;
    };
    struct State
    {
        bool paused = false, droneMoving = true, turbinesMoving = true,
             vehicleMoving = true;
        bool shadows = true, lighting = true, axes = true;
        int cameraMode = 0;
        float droneProgress = .3f, overviewAngle = .65f, overviewRadius = 42,
              overviewHeight = 19;
        float bladeDegrees = 0, vehicleX = -34, wheelDegrees = 0;
        float droneSpeed = .23f, turbineSpeed = 72.0f, vehicleSpeed = 6.2f;
        glm::vec3 sunPosition{-34.0f, 24.0f, -22.0f};
    };
    State state;
    int width = 1280, height = 720; // initial window width

    const char *vertexSource = R"GLSL(
#version 330 core
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
uniform mat4 model,view,projection;
uniform mat3 normalMatrix;
uniform vec3 lightPosition;
uniform float shadowPlaneY;
uniform bool shadowMode;
out vec3 worldPosition;
out vec3 normal;
void main(){
    // Change the local vertex into a world-space vertex.
    vec4 p=model*vec4(aPosition,1.0);
    if(shadowMode){
        // Project this vertex from the sun onto the flat ground.
        float t=(shadowPlaneY-lightPosition.y)/(p.y-lightPosition.y);
        p.xyz=lightPosition+t*(p.xyz-lightPosition);p.y+=.018;
    }
    // Send world information to the fragment shader for lighting.
    worldPosition=p.xyz;normal=normalize(normalMatrix*aNormal);
    // Apply the Model-View-Projection transformation.
    gl_Position=projection*view*p;
}
)GLSL";

    const char *fragmentSource = R"GLSL(
#version 330 core
in vec3 worldPosition;
in vec3 normal;
uniform vec3 objectColor,lightPosition,cameraPosition;
uniform bool lightingEnabled,shadowMode,emissive;
out vec4 color;
void main(){
    // In shadow mode, draw the object as a transparent dark shape.
    if(shadowMode){color=vec4(.022,.025,.03,.48);return;}
    if(emissive||!lightingEnabled){color=vec4(objectColor,1);return;}
    // N, L, V and H mean normal, light, view and halfway vectors.
    vec3 N=normalize(normal),L=normalize(lightPosition-worldPosition);
    vec3 V=normalize(cameraPosition-worldPosition),H=normalize(L+V);
    float diff=max(dot(N,L),0.0),spec=pow(max(dot(N,H),0.0),44.0);
    float d=length(lightPosition-worldPosition);
    // Make the light weaker when the object is farther from the sun.
    float attenuation=1.0/(1.0+.01*d+.0006*d*d);
    // Combine ambient, diffuse and shiny specular lighting.
    color=vec4(.24*objectColor+attenuation*(.94*diff*objectColor+vec3(.52)*spec),1);
}
)GLSL";

    // Prints shader errors to the terminal when compilation fails.
    void showLog(GLuint o, bool program)
    {
        GLint n = 0;
        // Read the length of a program-linker error message.
        if (program)
            glGetProgramiv(o, GL_INFO_LOG_LENGTH, &n);
        // Read the length of a shader-compiler error message.
        else
            glGetShaderiv(o, GL_INFO_LOG_LENGTH, &n);
        if (n <= 1)
            return;
        std::string log(static_cast<std::size_t>(n), '\0');
        // Copy a program-linker message from OpenGL into the string.
        if (program)
            glGetProgramInfoLog(o, n, nullptr, log.data());
        // Copy a shader-compiler message from OpenGL into the string.
        else
            glGetShaderInfoLog(o, n, nullptr, log.data());
        std::cerr << log << '\n';
    }
    GLuint makeProgram()
    {
        auto compile = [](GLenum type, const char *s)
        {
            // Create an empty GPU shader object of the requested type.
            GLuint x = glCreateShader(type);
            // Give the GLSL source code to the shader object.
            glShaderSource(x, 1, &s, nullptr);
            // Compile the GLSL source into GPU instructions.
            glCompileShader(x);
            GLint ok = GL_FALSE;
            // Check whether shader compilation succeeded.
            glGetShaderiv(x, GL_COMPILE_STATUS, &ok);
            if (!ok)
            {
                showLog(x, false);
                // Release the unusable shader after a compilation failure.
                glDeleteShader(x);
                return GLuint{0};
            }
            return x;
        };
        GLuint vs = compile(GL_VERTEX_SHADER, vertexSource),
               fs = compile(GL_FRAGMENT_SHADER, fragmentSource);
        if (!vs || !fs)
            return 0;
        // Create a program that can combine multiple compiled shaders.
        GLuint p = glCreateProgram();
        // Add the vertex shader to the new program.
        glAttachShader(p, vs);
        // Add the fragment shader to the new program.
        glAttachShader(p, fs);
        // Link both shaders into one executable rendering program.
        glLinkProgram(p);
        glDeleteShader(vs);
        glDeleteShader(fs);
        GLint ok = GL_FALSE;
        // Check whether the shader program linked successfully.
        glGetProgramiv(p, GL_LINK_STATUS, &ok);
        if (!ok)
        {
            showLog(p, true);
            // Release the invalid program after a linking failure.
            glDeleteProgram(p);
            return 0;
        }
        return p;
    }

    Mesh upload(const std::vector<float> &v, const std::vector<unsigned int> &i)
    {
        Mesh m;
        m.count = static_cast<GLsizei>(i.size());
        // Create a Vertex Array Object that remembers the mesh layout.
        glGenVertexArrays(1, &m.vao);
        // Create one buffer for vertex positions and normals.
        glGenBuffers(1, &m.vbo);
        // Create one buffer for triangle vertex indices.
        glGenBuffers(1, &m.ebo);
        // Make this mesh's Vertex Array Object active.
        glBindVertexArray(m.vao);
        // Select the vertex buffer as the current array buffer.
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
        // Copy all position and normal values into GPU memory.
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(v.size() * sizeof(float)), v.data(),
                     GL_STATIC_DRAW);
        // Select the index buffer used to assemble triangles.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
        // Copy the triangle indices into GPU memory.
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(i.size() * sizeof(unsigned int)),
                     i.data(), GL_STATIC_DRAW);
        // Describe attribute 0 as three position floats in each vertex.
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
        // Allow the vertex shader to read position attribute 0.
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              reinterpret_cast<void *>(3 * sizeof(float)));
        // Allow the vertex shader to read normal attribute 1.
        glEnableVertexAttribArray(1);
        // Unbind the mesh so later setup cannot change it accidentally.
        glBindVertexArray(0);
        return m;
    }
    // Creates a reusable cube from triangles.
    Mesh makeCube()
    {
        std::vector<float> v = {
            -.5f, -.5f, .5f, 0, 0, 1, .5f, -.5f, .5f, 0, 0, 1,
            .5f, .5f, .5f, 0, 0, 1, -.5f, .5f, .5f, 0, 0, 1,
            .5f, -.5f, -.5f, 0, 0, -1, -.5f, -.5f, -.5f, 0, 0, -1,
            -.5f, .5f, -.5f, 0, 0, -1, .5f, .5f, -.5f, 0, 0, -1,
            -.5f, -.5f, -.5f, -1, 0, 0, -.5f, -.5f, .5f, -1, 0, 0,
            -.5f, .5f, .5f, -1, 0, 0, -.5f, .5f, -.5f, -1, 0, 0,
            .5f, -.5f, .5f, 1, 0, 0, .5f, -.5f, -.5f, 1, 0, 0,
            .5f, .5f, -.5f, 1, 0, 0, .5f, .5f, .5f, 1, 0, 0,
            -.5f, .5f, .5f, 0, 1, 0, .5f, .5f, .5f, 0, 1, 0,
            .5f, .5f, -.5f, 0, 1, 0, -.5f, .5f, -.5f, 0, 1, 0,
            -.5f, -.5f, -.5f, 0, -1, 0, .5f, -.5f, -.5f, 0, -1, 0,
            .5f, -.5f, .5f, 0, -1, 0, -.5f, -.5f, .5f, 0, -1, 0};
        std::vector<unsigned int> i = {
            0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 8, 9, 10, 10, 11, 8,
            12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};
        return upload(v, i);
    }
    Mesh makeCylinder(int segments, float topRadius = 1)
    {
        std::vector<float> v;
        std::vector<unsigned int> idx;
        float slope = 1 - topRadius;
        for (int n = 0; n <= segments; ++n)
        {
            float a = 2 * PI * n / segments, c = std::cos(a), s = std::sin(a);
            glm::vec3 q = glm::normalize(glm::vec3(c, slope, s));
            v.insert(v.end(), {c, 0, s, q.x, q.y, q.z, topRadius * c, 1, topRadius * s,
                               q.x, q.y, q.z});
        }
        for (int n = 0; n < segments; ++n)
        {
            unsigned int k = static_cast<unsigned int>(2 * n);
            idx.insert(idx.end(), {k, k + 1, k + 2, k + 1, k + 3, k + 2});
        }
        auto cap = [&](float y, float r, float ny, bool reverse)
        {
            unsigned int c = static_cast<unsigned int>(v.size() / 6);
            v.insert(v.end(), {0, y, 0, 0, ny, 0});
            unsigned int rim = c + 1;
            for (int n = 0; n <= segments; ++n)
            {
                float a = 2 * PI * n / segments;
                v.insert(v.end(), {r * std::cos(a), y, r * std::sin(a), 0, ny, 0});
            }
            for (int n = 0; n < segments; ++n)
            {
                unsigned int a = rim + static_cast<unsigned int>(n), b = a + 1;
                if (reverse)
                    idx.insert(idx.end(), {c, b, a});
                else
                    idx.insert(idx.end(), {c, a, b});
            }
        };
        cap(0, 1, -1, false);
        cap(1, topRadius, 1, true);
        return upload(v, idx);
    }
    // Creates a sphere using horizontal stacks and vertical slices.
    Mesh makeSphere(int stacks, int slices)
    {
        std::vector<float> v;
        std::vector<unsigned int> idx;
        for (int i = 0; i <= stacks; ++i)
        {
            float p = PI * i / stacks;
            for (int j = 0; j <= slices; ++j)
            {
                float t = 2 * PI * j / slices;
                glm::vec3 q(std::sin(p) * std::cos(t), std::cos(p),
                            std::sin(p) * std::sin(t));
                v.insert(v.end(), {q.x, q.y, q.z, q.x, q.y, q.z});
            }
        }
        for (int i = 0; i < stacks; ++i)
            for (int j = 0; j < slices; ++j)
            {
                unsigned int a = static_cast<unsigned int>(i * (slices + 1) + j);
                unsigned int b = a + static_cast<unsigned int>(slices + 1);
                idx.insert(idx.end(), {a, b, a + 1, a + 1, b, b + 1});
            }
        return upload(v, idx);
    }
    // Creates a model matrix: translate, then rotate, then scale.
    glm::mat4 trs(glm::vec3 p, glm::vec3 r, glm::vec3 s)
    {
        glm::mat4 m = glm::translate(glm::mat4(1), p);
        // Rotate around Y, then X, then Z to orient the object.
        m = glm::rotate(m, glm::radians(r.y), {0, 1, 0});
        m = glm::rotate(m, glm::radians(r.x), {1, 0, 0});
        m = glm::rotate(m, glm::radians(r.z), {0, 0, 1});
        // Scale the oriented object to its final size.
        return glm::scale(m, s);
    }
    void setBool(GLuint p, const char *n, bool v)
    {
        // Send a C++ Boolean to the named GLSL uniform as 0 or 1.
        glUniform1i(glGetUniformLocation(p, n), v ? 1 : 0);
    }
    void draw(GLuint p, const Mesh &m, const glm::mat4 &model, glm::vec3 colour,
              bool emissive = false)
    {
        // Send the object's world transformation to the vertex shader.
        glUniformMatrix4fv(glGetUniformLocation(p, "model"), 1, GL_FALSE,
                           glm::value_ptr(model));
        glm::mat3 normal = glm::transpose(glm::inverse(glm::mat3(model)));
        glUniformMatrix3fv(glGetUniformLocation(p, "normalMatrix"), 1, GL_FALSE,
                           glm::value_ptr(normal));
        // Send the object's base RGB colour to the fragment shader.
        glUniform3fv(glGetUniformLocation(p, "objectColor"), 1,
                     glm::value_ptr(colour));
        setBool(p, "emissive", emissive);
        // Select the geometry stored for this mesh.
        glBindVertexArray(m.vao);
        // Draw every indexed triangle in the selected mesh.
        glDrawElements(GL_TRIANGLES, m.count, GL_UNSIGNED_INT, nullptr);
    }
    // Updates the drawing area whenever the window size changes.
    void resize(GLFWwindow *, int w, int h)
    {
        width = std::max(w, 1);
        height = std::max(h, 1);
        // Match OpenGL's drawing viewport to the resized framebuffer.
        glViewport(0, 0, width, height);
    }
    void key(GLFWwindow *w, int code, int, int action, int)
    {
        if (action != GLFW_PRESS)
            return;
        if (ImGui::GetCurrentContext() != nullptr &&
            ImGui::GetIO().WantCaptureKeyboard)
            return;
        switch (code)
        {
        case GLFW_KEY_ESCAPE:
            // Mark the window for closure so the main loop ends safely.
            glfwSetWindowShouldClose(w, GL_TRUE);
            break;
        case GLFW_KEY_SPACE:
            state.paused = !state.paused;
            break;
        case GLFW_KEY_C:
            state.cameraMode = (state.cameraMode + 1) % 3;
            break;
        case GLFW_KEY_D:
            state.droneMoving = !state.droneMoving;
            break;
        case GLFW_KEY_T:
            state.turbinesMoving = !state.turbinesMoving;
            break;
        case GLFW_KEY_V:
            state.vehicleMoving = !state.vehicleMoving;
            break;
        case GLFW_KEY_H:
            state.shadows = !state.shadows;
            break;
        case GLFW_KEY_L:
            state.lighting = !state.lighting;
            break;
        case GLFW_KEY_A:
            state.axes = !state.axes;
            break;
        case GLFW_KEY_R:
            state = State{};
            break;
        default:
            break;
        }
    }
    // Frees the GPU memory used by one mesh.
    void destroy(Mesh &m)
    {
        // Release the mesh layout object stored on the GPU.
        glDeleteVertexArrays(1, &m.vao);
        // Release the vertex-data GPU buffer.
        glDeleteBuffers(1, &m.vbo);
        // Release the triangle-index GPU buffer.
        glDeleteBuffers(1, &m.ebo);
        m = {};
    }

} // namespace

int main()
{
    if (!glfwInit())
        return EXIT_FAILURE;
    // Request OpenGL major version 3.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // Request OpenGL minor version 3, giving an OpenGL 3.3 context.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Use only the modern Core Profile API.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // Ask for four samples per pixel to smooth jagged edges.
    glfwWindowHint(GLFW_SAMPLES, 4);
    // Create the application window and its OpenGL rendering context.
    GLFWwindow *window = glfwCreateWindow(
        width, height, "Wind Turbine Farm Drone Tour", nullptr, nullptr);
    if (!window)
    {
        // Shut down GLFW if creating the window failed.
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    // Call resize() whenever the framebuffer dimensions change.
    glfwSetFramebufferSizeCallback(window, resize);
    // Call key() whenever GLFW receives a keyboard event.
    glfwSetKeyCallback(window, key);
    glfwGetFramebufferSize(window, &width, &height);
    // Define the initial pixel area in which OpenGL may render.
    glViewport(0, 0, width, height);

    // Compile and link the shaders before creating the scene objects.
    GLuint program = makeProgram();
    if (!program)
    {
        // Destroy the successfully created window after a shader failure.
        glfwDestroyWindow(window);
        // Release GLFW resources before exiting after the failure.
        glfwTerminate();
        return EXIT_FAILURE;
    }
    // Connect Dear ImGui to this window and OpenGL context.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Access ImGui configuration and frame statistics.
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // Apply ImGui's built-in dark colour theme.
    ImGui::StyleColorsDark();
    // Access the style values so the controls can be rounded.
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 5.0f;
    style.GrabRounding = 5.0f;
    style.WindowBorderSize = 1.0f;
    // Connect ImGui input handling to the GLFW window.
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    // Connect ImGui drawing to OpenGL using GLSL version 330.
    ImGui_ImplOpenGL3_Init("#version 330 core");
    Mesh cube = makeCube(), cylinder = makeCylinder(40),
         tower = makeCylinder(48, .42f), cone = makeCylinder(40, .02f),
         sphere = makeSphere(20, 32);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    // Set the sky-blue colour used to erase the previous frame.
    glClearColor(.48f, .73f, .91f, 1);
    const float wheelRadius = .38f;
    const glm::vec3 turbinePositions[] = {{-15, 0, -11}, {0, 0, -14}, {15, 0, -9}, {-13, 0, 10}, {3, 0, 7}, {17, 0, 13}};
    // Read GLFW's timer to establish the first animation timestamp.
    double previous = glfwGetTime();
    std::cout << "Wind Turbine Farm\nC camera | D drone | T turbines | V vehicle "
                 "| H shadows | L light\n"
              << "Arrows camera | +/- zoom | A axes | R reset | Space pause\n";

    while (!glfwWindowShouldClose(window))
    {
        double now = glfwGetTime();
        float dt = std::min(static_cast<float>(now - previous), .05f);
        previous = now;
        // Process waiting keyboard, mouse, resize and window events.
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        // Collect this frame's input and window information from GLFW.
        ImGui_ImplGlfw_NewFrame();
        // Begin constructing the ImGui widgets for this frame.
        ImGui::NewFrame();
        // Change animation values only when the scene is not paused.
        if (!state.paused)
        {
            // Move the drone forward along its mathematical route.
            if (state.droneMoving)
                state.droneProgress += state.droneSpeed * dt;

            // Increase the shared turbine-blade rotation angle.
            if (state.turbinesMoving)
                state.bladeDegrees += state.turbineSpeed * dt;

            if (state.vehicleMoving)
            {
                state.vehicleX += state.vehicleSpeed * dt;
                state.wheelDegrees -=
                    glm::degrees((state.vehicleSpeed * dt) / wheelRadius);
                if (state.vehicleX > 36)
                    state.vehicleX = -36;
            }
            if (state.cameraMode == 1)
                state.overviewAngle += .12f * dt;
        }
        if (!ImGui::GetIO().WantCaptureKeyboard)
        {
            // Holding Left rotates the overview camera counter-clockwise.
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                state.overviewAngle -= .85f * dt;
            // Holding Right rotates the overview camera clockwise.
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                state.overviewAngle += .85f * dt;
            // Holding Up raises the overview camera.
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                state.overviewHeight += 7 * dt;
            // Holding Down lowers the overview camera.
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                state.overviewHeight -= 7 * dt;
            // Holding + reduces the orbit radius and zooms in.
            if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
                state.overviewRadius -= 8 * dt;
            // Holding - increases the orbit radius and zooms out.
            if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
                state.overviewRadius += 8 * dt;
        }
        state.overviewHeight = std::clamp(state.overviewHeight, 7.0f, 30.0f);
        state.overviewRadius = std::clamp(state.overviewRadius, 24.0f, 58.0f);

        auto dronePoint = [](float p)
        {
            return glm::vec3(18 * std::sin(p), 8.5f + 1.7f * std::sin(2 * p),
                             17 * std::sin(.5f * p));
        };
        glm::vec3 vehiclePosition(state.vehicleX, .25f, 0), camera, targetPoint;
        if (state.cameraMode == 0)
        {
            camera = dronePoint(state.droneProgress);
            targetPoint =
                dronePoint(state.droneProgress + .12f) + glm::vec3(0, -1, 0);
        }
        // Camera mode 1: orbit around the complete farm.
        else if (state.cameraMode == 1)
        {
            camera = {state.overviewRadius * std::cos(state.overviewAngle),
                      state.overviewHeight,
                      state.overviewRadius * std::sin(state.overviewAngle)};
            targetPoint = {0, 5, 0};
        }
        // Camera mode 2: follow the service vehicle from behind.
        else
        {
            camera = vehiclePosition + glm::vec3(-9, 4.5f, 7);
            targetPoint = vehiclePosition + glm::vec3(5, 1, 0);
        }
        ImGui::SetNextWindowPos(ImVec2(18, 18), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(350, 0), ImGuiCond_FirstUseEver);
        ImGui::Begin("Wind Farm Control Panel", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize);
        // Display the green heading at the top of the panel.
        ImGui::TextColored(ImVec4(.45f, .90f, .55f, 1), "REAL-TIME SCENE CONTROLS");
        ImGui::Separator();
        // Create a button whose label and action reflect the pause state.
        if (ImGui::Button(state.paused ? "Resume all" : "Pause all",
                          ImVec2(155, 0)))
            state.paused = !state.paused;
        // Place the reset button beside the pause button.
        ImGui::SameLine();
        // Restore every setting when the user presses Reset scene.
        if (ImGui::Button("Reset scene", ImVec2(155, 0)))
            state = State{};

        if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Bind this checkbox directly to drone movement.
            ImGui::Checkbox("Drone movement", &state.droneMoving);
            // Let the user adjust how quickly route progress changes.
            ImGui::SliderFloat("Drone speed", &state.droneSpeed, .03f, .65f, "%.2f");
            // Bind this checkbox directly to turbine rotation.
            ImGui::Checkbox("Turbine rotation", &state.turbinesMoving);
            // Let the user set rotor speed in degrees per second.
            ImGui::SliderFloat("Turbine speed", &state.turbineSpeed, 0, 180,
                               "%.0f deg/s");
            // Bind this checkbox directly to vehicle movement.
            ImGui::Checkbox("Vehicle movement", &state.vehicleMoving);
            // Let the user set vehicle speed in world units per second.
            ImGui::SliderFloat("Vehicle speed", &state.vehicleSpeed, 0, 14,
                               "%.1f units/s");
        }
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const char *modes[] = {"Drone route", "Overview orbit", "Vehicle follow"};
            // Present the three camera modes in a drop-down list.
            ImGui::Combo("Camera mode", &state.cameraMode, modes, 3);
            if (state.cameraMode == 1)
            {
                // Adjust the vertical position of the overview camera.
                ImGui::SliderFloat("Orbit height", &state.overviewHeight, 7, 30,
                                   "%.1f");
                // Adjust the overview camera's distance from the farm centre.
                ImGui::SliderFloat("Orbit radius", &state.overviewRadius, 24, 58,
                                   "%.1f");
            }
        }
        if (ImGui::CollapsingHeader("Lighting and shadows",
                                    ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Turn the fragment shader's lighting calculation on or off.
            ImGui::Checkbox("Enable lighting", &state.lighting);
            // Place the shadows checkbox on the same row.
            ImGui::SameLine();
            // Turn the separate projected-shadow drawing pass on or off.
            ImGui::Checkbox("Enable shadows", &state.shadows);
            // Move the light source left or right in world space.
            ImGui::SliderFloat("Sun X", &state.sunPosition.x, -50, 50, "%.1f");
            // Raise or lower the light source above the ground.
            ImGui::SliderFloat("Sun height", &state.sunPosition.y, 18, 45, "%.1f");
            // Move the light source across the farm in world space.
            ImGui::SliderFloat("Sun Z", &state.sunPosition.z, -50, 50, "%.1f");
        }
        if (ImGui::CollapsingHeader("Live coordinates",
                                    ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Print the camera's current world coordinates.
            ImGui::Text("Camera  X:%6.1f  Y:%5.1f  Z:%6.1f", camera.x, camera.y,
                        camera.z);
            // Print the service vehicle's current world coordinates.
            ImGui::Text("Vehicle X:%6.1f  Y:%5.1f  Z:%6.1f", vehiclePosition.x,
                        vehiclePosition.y, vehiclePosition.z);
            // Print the rotor angle wrapped into one 360-degree revolution.
            ImGui::Text("Blade angle: %6.1f degrees",
                        std::fmod(state.bladeDegrees, 360.0f));
            // Print ImGui's measured frames-per-second value.
            ImGui::Text("Frame rate: %.1f FPS", ImGui::GetIO().Framerate);
        }
        // Draw a line above the final global controls.
        ImGui::Separator();
        // Let the user show or hide the coloured coordinate axes.
        ImGui::Checkbox("Show coordinate axes", &state.axes);
        // Show the matrix order as muted instructional text.
        ImGui::TextDisabled("MVP: clip = Projection x View x Model x vertex");
        // Finish the current ImGui window.
        ImGui::End();

        glm::mat4 view = glm::lookAt(camera, targetPoint, {0, 1, 0});

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f), static_cast<float>(width) / height, .1f, 140.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Activate this shader program for all following draw calls.
        glUseProgram(program);
        // Send the camera view matrix to the vertex shader.
        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        // Send the perspective projection matrix to the vertex shader.
        glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE,
                           glm::value_ptr(projection));
        // Send the movable sun position to both shaders.
        glUniform3fv(glGetUniformLocation(program, "lightPosition"), 1,
                     glm::value_ptr(state.sunPosition));
        glUniform3fv(glGetUniformLocation(program, "cameraPosition"), 1,
                     glm::value_ptr(camera));
        // Set the flat Y level onto which object shadows are projected.
        glUniform1f(glGetUniformLocation(program, "shadowPlaneY"), .065f);
        // Tell the fragment shader whether it should calculate lighting.
        setBool(program, "lightingEnabled", state.lighting);
        // Begin in normal colour mode rather than shadow mode.
        setBool(program, "shadowMode", false);

        // Draw the farm ground and all static scenery.
        auto terrain = [&]()
        {
            // A grass field supports a  road with dashed centre markings.
            draw(program, cube, trs({0, -.25f, 0}, {0, 0, 0}, {82, .5f, 66}),
                 {.21f, .47f, .18f});
            draw(program, cube, trs({0, .02f, 0}, {0, 0, 0}, {76, .10f, 4.8f}),
                 {.22f, .23f, .22f});
            for (int x = -34; x <= 34; x += 7)
                draw(program, cube,
                     trs({static_cast<float>(x), .10f, 0}, {0, 0, 0},
                         {3.4f, .025f, .16f}),
                     {.91f, .82f, .28f}, true);
            // Layered cones form the distant mountain range and snowy peaks.
            for (int i = 0; i < 7; ++i)
            {
                float x = -32 + i * 11.0f, z = 27 + (i % 2) * 3.0f;
                draw(program, cone,
                     trs({x, 0, z}, {0, 0, 0},
                         {7.5f + (i % 3), 10.5f + (i % 2) * 2, 6.5f + (i % 2)}),
                     {.31f, .38f, .30f});
                draw(program, cone,
                     trs({x, 7.3f + (i % 2) * 1.4f, z}, {0, 0, 0}, {2.4f, 3.5f, 2.2f}),
                     {.86f, .88f, .86f});
            }
            // Two boxes form a simple farm building and its roof.
            draw(program, cube, trs({-27, 2.0f, -23}, {0, 0, 0}, {10, 4, 7}),
                 {.58f, .45f, .28f});
            draw(program, cube, trs({-27, 4.25f, -23}, {0, 0, 0}, {11, .55f, 8}),
                 {.24f, .16f, .10f});
            // Cylinders and spheres create rows of trees around the farm.
            for (int i = 0; i < 12; ++i)
            {
                float x = -34 + i * 6, z = (i % 2 ? -26.0f : 22.0f);
                draw(program, cylinder, trs({x, 0, z}, {0, 0, 0}, {.15f, 1.3f, .15f}),
                     {.29f, .15f, .06f});
                draw(program, sphere, trs({x, 1.7f, z}, {0, 0, 0}, {.85f, 1.15f, .85f}),
                     {.06f, .36f, .09f});
            }
        };
        auto turbines = [&]()
        {
            for (int index = 0; index < 6; ++index)
            {
                glm::vec3 p = turbinePositions[index];
                draw(program, tower, trs(p, {0, 0, 0}, {.72f, 10.5f, .72f}),
                     {.80f, .82f, .81f});
                draw(program, cube,
                     trs(p + glm::vec3(0, 10.55f, -.25f), {0, 0, 0},
                         {1.25f, .72f, 2.0f}),
                     {.72f, .75f, .75f});
                glm::mat4 rotor =
                    glm::translate(glm::mat4(1), p + glm::vec3(0, 10.55f, -1.32f)) *
                    glm::rotate(glm::mat4(1),
                                glm::radians(state.bladeDegrees + index * 13.0f),
                                {0, 0, 1});
                draw(program, sphere,
                     rotor * trs({0, 0, 0}, {0, 0, 0}, {.48f, .48f, .38f}),
                     {.90f, .91f, .88f});
                for (int blade = 0; blade < 3; ++blade)
                {
                    glm::mat4 b =
                        rotor * glm::rotate(glm::mat4(1), glm::radians(blade * 120.0f),
                                            {0, 0, 1});
                    draw(program, cube,
                         b * trs({0, 1.95f, 0}, {0, 0, -4}, {.30f, 3.9f, .15f}),
                         {.88f, .89f, .86f});
                }
                draw(program, sphere,
                     trs(p + glm::vec3(0, 10.55f, -1.72f), {0, 0, 0},
                         {.12f, .12f, .12f}),
                     {1, .08f, .04f}, true);
            }
        };
        glm::mat4 vehicleParent = glm::translate(glm::mat4(1), vehiclePosition);
        auto vehicle = [&]()
        {
            draw(program, cube,
                 vehicleParent * trs({0, .56f, 0}, {0, 0, 0}, {3.0f, .62f, 1.45f}),
                 {.94f, .48f, .05f});
            draw(program, cube,
                 vehicleParent *
                     trs({-.35f, 1.08f, 0}, {0, 0, 0}, {1.45f, .58f, 1.18f}),
                 {.14f, .27f, .34f});
            draw(program, cube,
                 vehicleParent * trs({1.5f, .65f, 0}, {0, 0, 0}, {.08f, .22f, 1.0f}),
                 {1, .90f, .45f}, true);
            for (float x : {-.95f, .95f})
                for (float z : {-.77f, .77f})
                {
                    glm::mat4 w =
                        vehicleParent * glm::translate(glm::mat4(1), {x, .40f, z}) *
                        glm::rotate(glm::mat4(1), glm::radians(state.wheelDegrees),
                                    {0, 0, 1}) *
                        glm::rotate(glm::mat4(1), glm::radians(90.0f), {1, 0, 0}) *
                        glm::scale(glm::mat4(1), {wheelRadius, .28f, wheelRadius});
                    draw(program, cylinder, w, {.035f, .035f, .04f});
                }
        };
        // Draw red X, green Y and blue Z axes as an orientation guide.
        auto axes = [&]()
        {
            glm::vec3 o(-36, .2f, -27);
            draw(program, cube,
                 trs(o + glm::vec3(1.5f, 0, 0), {0, 0, 0}, {3, .07f, .07f}),
                 {1, 0, 0}, true);
            draw(program, cube,
                 trs(o + glm::vec3(0, 1.5f, 0), {0, 0, 0}, {.07f, 3, .07f}),
                 {0, 1, 0}, true);
            draw(program, cube,
                 trs(o + glm::vec3(0, 0, 1.5f), {0, 0, 0}, {.07f, .07f, 3}),
                 {0, 0, 1}, true);
        };

        terrain();
        // Draw every animated turbine once in normal colour.
        turbines();
        // Draw the moving service vehicle once in normal colour.
        vehicle();
        // Draw the orientation guide only when the user enables it.
        if (state.axes)
            axes();
        // A bright sphere shows the light source's current position.
        draw(program, sphere, trs(state.sunPosition, {0, 0, 0}, {.82f, .82f, .82f}),
             {1, .70f, .11f}, true);
        if (state.shadows)
        {
            setBool(program, "shadowMode", true);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            // Redraw turbine geometry as projected shadows.
            turbines();
            // Redraw vehicle geometry as a projected shadow.
            vehicle();
            // Restore depth writes for normal rendering in the next frame.
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            // Restore normal shader mode after the shadow pass.
            setBool(program, "shadowMode", false);
        }
        // Convert all control-panel widgets into final ImGui draw data.
        ImGui::Render();
        // Draw the ImGui panel over the completed 3D scene.
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    // Release resources owned by ImGui's OpenGL renderer.
    ImGui_ImplOpenGL3_Shutdown();
    // Disconnect ImGui input handling from GLFW.
    ImGui_ImplGlfw_Shutdown();
    // Destroy ImGui's global context and widget state.
    ImGui::DestroyContext();
    // Release every reusable mesh and its GPU buffers.
    destroy(cube);
    destroy(cylinder);
    destroy(tower);
    destroy(cone);
    destroy(sphere);
    // Release the linked shader program from GPU memory.
    glDeleteProgram(program);
    // Destroy the application window and its OpenGL context.
    glfwDestroyWindow(window);
    // Release all remaining GLFW resources.
    glfwTerminate();
    return EXIT_SUCCESS;
}
