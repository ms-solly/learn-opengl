#define CGLTF_IMPLEMENTATION
#include <cgltf/cgltf.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Shader sources
const char* vertex_shader_source = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)glsl";

const char* fragment_shader_source = R"glsl(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.8, 0.8, 0.8, 1.0); // Default gray color
}
)glsl";

GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    // Error checking
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation error: %s\n", infoLog);
    }
    return shader;
}

GLuint create_shader_program() {
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    // Error checking
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("Shader linking error: %s\n", infoLog);
    }
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    return program;
}

cgltf_data* load_gltf(const char* filename) {
    cgltf_options options = {};
    cgltf_data* data = NULL;
    
    if (cgltf_parse_file(&options, filename, &data) != cgltf_result_success) {
        printf("Failed to parse GLTF file\n");
        return NULL;
    }
    
    if (cgltf_load_buffers(&options, data, filename) != cgltf_result_success) {
        printf("Failed to load GLTF buffers\n");
        cgltf_free(data);
        return NULL;
    }
    
    if (cgltf_validate(data) != cgltf_result_success) {
        printf("GLTF validation warnings\n");
        // Continue anyway as validation might report non-critical issues
    }
    
    return data;
}

double lastX = 0, lastY = 0;
float rotX = 0, rotY = 0;
bool firstMouse = true;

void cursor_pos(GLFWwindow* window, double x, double y) {
    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
        return;
    }
    rotY += (x - lastX) * 0.3f;
    rotX += (y - lastY) * 0.3f;
    lastX = x;
    lastY = y;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(1200, 800, "GLTF Viewer", NULL, NULL);
    if (!window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        printf("Failed to initialize GLEW\n");
        glfwTerminate();
        return -1;
    }
    
    // Set up callbacks
    glfwSetCursorPosCallback(window, cursor_pos);
    
    // Configure OpenGL
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    // Create shader program
    GLuint shader_program = create_shader_program();
    glUseProgram(shader_program);
    
    // Load GLTF model
    cgltf_data* gltf = load_gltf("../assets/bench_01.glb");
    if (!gltf || gltf->meshes_count == 0) {
        printf("Failed to load GLTF file or no meshes found\n");
        glfwTerminate();
        return 1;
    }
    
    // Get the first mesh and primitive
    cgltf_mesh* mesh = &gltf->meshes[0];
    cgltf_primitive* prim = &mesh->primitives[0];
    
    // Extract position data
    float* positions = NULL;
    unsigned short* indices = NULL;
    int num_positions = 0;
    int num_indices = 0;
    
    for (cgltf_size i = 0; i < prim->attributes_count; ++i) {
        cgltf_attribute* attr = &prim->attributes[i];
        if (attr->type == cgltf_attribute_type_position) {
            cgltf_accessor* acc = attr->data;
            positions = (float*)((uint8_t*)acc->buffer_view->buffer->data + 
                               acc->buffer_view->offset + 
                               acc->offset);
            num_positions = acc->count;
        }
    }
    
    if (prim->indices) {
        cgltf_accessor* acc = prim->indices;
        indices = (unsigned short*)((uint8_t*)acc->buffer_view->buffer->data + 
                                   acc->buffer_view->offset + 
                                   acc->offset);
        num_indices = acc->count;
    }
    
    // Create VAO, VBO, and EBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    // Position attribute
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, num_positions * 3 * sizeof(float), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(unsigned short), indices, GL_STATIC_DRAW);
    
    // Get uniform locations
    GLint model_loc = glGetUniformLocation(shader_program, "model");
    GLint view_loc = glGetUniformLocation(shader_program, "view");
    GLint proj_loc = glGetUniformLocation(shader_program, "projection");
    
    // Projection matrix
    glm::mat4 projection = glm::perspective(
        glm::radians(40.0f), 
        (float)1200 / (float)800, 
        0.1f, 
        100.0f
    );
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(projection));
    
    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // View matrix (camera)
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
        
        // Model matrix (object transformation)
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        
        // Draw the mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, 0);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shader_program);
    cgltf_free(gltf);
    glfwTerminate();
    
    return 0;
}
