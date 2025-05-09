#include<GL/glew.h>
#include<GL/gl.h>
#include<GLFW/glfw3.h>
#include<stdio.h>
#include<math.h>


const char* vertexShaderSource = 
    "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "uniform vec2 uCursorPos;\n"
    "uniform vec2 uScreenSize;\n"
    "void main() {\n"
    "   vec2 normalizedPos = (aPos + uCursorPos) / uScreenSize * 2.0 - 1.0;\n"
    "   gl_Position = vec4(normalizedPos.x, -normalizedPos.y, 0.0, 1.0);\n"
    "}\0";

// Fragment shader modification
const char* fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec3 uColor;\n"
    "uniform int uClicked;\n" // New uniform
    "void main() {\n"
    "       FragColor = vec4(uColor, 1.0);\n"

    "}\0";

// Mouse position
double cursorPos[] = {0.0f, 0.0f};

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    cursorPos[0] = (float)xpos;
    cursorPos[1] = (float)ypos;
}



void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  
int main()
{

    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE,GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    if (window == NULL){
        fprintf(stderr,"Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    glewExperimental = GL_TRUE; // required for core profile
    GLenum glewInitResult = glewInit();
    if (glewInitResult != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(glewInitResult));
        return -1;
    } 


    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Cursor geometry (circle)
    float radius = 15.0f;
    int segments = 32;
    float vertices[segments*2 + 2]; // +2 for center point
    vertices[0] = 0.0f; vertices[1] = 0.0f; // Center
    
    for(int i = 0; i < segments; i++) {
        float angle = 2.0f * M_PI * (float)i / (float)segments;
        vertices[2 + i*2] = cos(angle) * radius;
        vertices[3 + i*2] = sin(angle) * radius;
    }

    // Create VAO, VBO
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  




    
while(!glfwWindowShouldClose(window))
{

 glClear(GL_COLOR_BUFFER_BIT);

        // Get window size
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // Draw cursor
        glUseProgram(shaderProgram);
        glUniform2f(glGetUniformLocation(shaderProgram, "uCursorPos"), cursorPos[0], cursorPos[1]);
        glUniform2f(glGetUniformLocation(shaderProgram, "uScreenSize"), (float)width, (float)height);
        glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), 1.0f, 0.0f, 0.0f); // Red
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 1); // +1 for center point

        // Add crosshair (optional)
        // Would require separate VBO with line geometry
       
 

        
    glfwSwapBuffers(window);
    glfwPollEvents();    
}
    


  glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
glfwTerminate();
return 0;







}
