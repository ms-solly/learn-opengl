#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

// types

typedef struct vec2{
    float x,y;
} vec2;

typedef struct vec3{
    float x, y, z;
} vec3;

typedef struct vec4{
    float x, y, z, w;
}vec4;

typedef struct mat4{
    float m[16];
}mat4;

typedef struct quat{
    float x, y, z, w;
}quat;
// transform components
typedef struct Transform{
    vec3 position;
    quat rotation;//euler angles
    vec3 scale;
    mat4 world_matrix;
    int parent_id;
    int child_id;
    int next_sibling;
    int prev_sibling;
} Transform;

// rendering components
typedef struct Vertex{
    vec3 position;
    vec3 normal;
    vec2 tex_coord;
    vec3 tangent;
    vec3 bitangent;
}Vertex;

typedef struct Mesh {
    GLuint VAO,VBO,EBO;
    int vertex_count;
    int index_count;
    Vertex* vertices;
    unsigned int* indices;
    int material_id;
} Mesh;

typedef struct Material{
    GLuint albebo_map;
    GLuint normal_map;
    GLuint metallic_roughness_map;
    vec4 base_color;
    float metallic;
    float roughness;
    float occlusion_strength;
}Material;

typedef struct ModelComponent{
    Mesh* mesh;
    Material* material;
    int transform_id;
    int draw_order;
}ModelComponent;

// lighting system
typedef enum Light_Type{
    LIGHT_DIRECTIONAL,
    LIGHT_POINT,
    LIGHT_SPOT
}LightType;

typedef struct Light{
    Light_Type type;
    vec3 color;
    float intesity;
    float range;
    float inner_cone_angle;
    float outer_cone_angle;
    int transform_id;
}Light;

// physics system
typedef enum Collider_Type{
    COLLIDER_SPHERE,
    COLLIDER_BOX,
    COLLIDER_CAPSULE,
    COLLIDER_MESH
}Collider_Type;

typedef struct Rigid_Body{
    vec3 linear_velocity;
    vec3 angular_velocity;
    float mass;
    float restitution;
    float friction;
    int collider_id;
}Rigid_Body;

typedef struct Collider {
    Collider_Type type;
    union {
        struct { float radius; } sphere;
        struct { vec3 half_extents; } box;
        struct { float radius, height; } capsule;
        struct { Mesh* collision_mesh; } mesh;
    };
    int transform_id;
} Collider;

// animation system 

ypedef struct Bone {
    Mat4 offset_matrix;
    int transform_id;
} Bone;

typedef struct Key_Frame {
    float timestamp;
    Vec3 translation;
    Quat rotation;
    Vec3 scale;
} Key_Frame;

typedef struct Animation_Channel {
    int bone_index;
    KeyFrame* key_frames;
    int num_key_frames;
} Animation_Channel;

typedef struct Animation {
    float duration;
    float ticks_per_second;
    AnimationChannel* channels;
    int num_channels;
} Animation;


// ui system 
typedef struct UI_Component {
    Vec2 position;
    Vec2 size;
    Vec4 color;
    GLuint texture;
    int z_order;
    char* text;
    void (*on_click)(void);
} UI_Component;

// camera system
typedef struct Camera {
    Mat4 view_matrix;
    Mat4 projection_matrix;
    float fov;
    float near_plane;
    float far_plane;
    int transform_id;
    int is_primary;
} Camera;

// input system 
typedef struct Input_State {
    struct {
        unsigned char current[512];
        unsigned char previous[512];
    } keys;
    
    struct {
        Vec2 position;
        Vec2 delta;
        unsigned char buttons[8];
        float scroll;
    } mouse;
    
    struct {
        float axes[16];
        unsigned char buttons[32];
    } gamepad;
} Input_State;

// resource management 
typedef struct Texture {
    GLuint id;
    int width;
    int height;
    int channels;
    char* path;
} Texture;

typedef struct Shader {
    GLuint program;
    struct {
        GLint model;
        GLint view;
        GLint projection;
        GLint base_color;
        // ... add more uniforms as needed
    } uniforms;
} Shader;

typedef struct Mesh_Manager {
    Mesh* meshes;
    int count;
    int capacity;
} Mesh_Manager;

// scene management 
typedef struct Entity {
    int id;
    unsigned int component_mask;
} Entity;

typedef struct Scene {
    Entity* entities;
    Transform* transforms;
    ModelComponent* models;
    Light* lights;
    Camera* cameras;
    RigidBody* physics_bodies;
    UIComponent* ui_elements;
    int entity_count;
} Scene;

// particle system 
typedef struct Particle {
    Vec3 position;
    Vec3 velocity;
    Vec4 color;
    float size;
    float lifetime;
    float age;
} Particle;

typedef struct Particle_Emitter {
    int max_particles;
    float emission_rate;
    Vec3 position;
    Vec3 velocity_range[2];
    Vec4 color_start;
    Vec4 color_end;
    Particle* particles;
} Particle_Emitter;

// audio system 
typedef struct Audio_Clip {
    unsigned int buffer_id;
    int channels;
    int sample_rate;
    size_t size;
    char* data;
} Audio_Clip;

typedef struct Audio_Source {
    unsigned int source_id;
    AudioClip* clip;
    Vec3 position;
    float volume;
    int is_looping;
} Audio_Source;

// debug system 
typedef struct Debug_Line {
    Vec3 start;
    Vec3 end;
    Vec4 color;
} Debug_Line;

typedef struct Debug_System {
    DebugLine* lines;
    int line_count;
    int line_capacity;
} Debug_System;

// Window dimensions
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};  

// Shader sources
const char* vertex_shader_source = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main() {\n"
    "   gl_Position =  vec4(aPos.x, aPos.y, aPos.z , 1.0);\n"
    "}\0";

const char* fragment_shader_source = "#version 330 core\n"
    "out vec4 Frag_Color;\n"
    "// uniform vec3 color;\n"
    "void main() {\n"
    "  // FragColor = vec4(color, 1.0);\n"
    "  Frag_Color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n" 
    "}\0";

// Function prototypes
void processInput(GLFWwindow* window);
int main() {
    // GLFW initialization
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Platformer", NULL, NULL);
    glfwMakeContextCurrent(window);

    glewExperimental = true;
    glewInit();

    unsigned int VBO;//vertex buffer objects
    glGenBuffers(1, &VBO);// gen buffer with ID
    glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind buffer to the object
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // copy data into currently bound buffer 

    unsigned int vertex_shader;// vertex shader
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);//gen vertex shader
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);// assign shafer source
    glCompileShader(vertex_shader);// compile vertex shader

    int  success;
    char info_log[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n %s\n",info_log);//vertex shader compilation failed
    }

    // fragment shader: this computes color output of pixels
    unsigned int fragment_shader;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    // shader program : linked version of combined multiple shaders 
    // compiled shaders ==linked to==>shader program object, and SPO is activated 
    // shader program which is activated its constituent shaders are used while rendering
    // shader1 output =is= shader2 input
    unsigned int shader_program;
    shader_program = glCreateProgram();// returns id reference to new program object 
    glAttachShader(shader_program, vertex_shader);//
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);// linked shader program
    
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, info_log);
        printf("ERROR::PROGRAM::SHADER::LINKING_FAILED\n %s\n", info_log);
    }
    glUseProgram(shader_program);//activates shader program
    //so every shader and rendering calls after this will use the above activated shader program

    while (!glfwWindowShouldClose(window)) {

        processInput(window);        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        


        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1,&VBO);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteBuffers(1,&VBO);
    glfwTerminate();
    return 0;
}

// Input handling
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

}


