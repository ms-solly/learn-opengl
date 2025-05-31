#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include<st>

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


