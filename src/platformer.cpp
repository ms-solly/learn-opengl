#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>


// Window dimensions
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Game state
typedef struct {
    float x, y;
    float width, height;
    float velocityX, velocityY;
    bool onGround;
} GameObject;

GameObject player = {0.0f, 0.0f, 0.1f, 0.2f, 0.0f, 0.0f, false};
GameObject platform = {0.0f, -0.8f, 0.5f, 0.1f};

// Shader sources
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "uniform mat4 projection;\n"
    "void main() {\n"
    "   gl_Position = projection * vec4(aPos, 0.0, 1.0);\n"
    "}\0";

const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec3 color;\n"
    "void main() {\n"
    "   FragColor = vec4(color, 1.0);\n"
    "}\0";

// Function prototypes
void processInput(GLFWwindow* window);
void updateGame(float deltaTime);
void renderObject(GameObject obj, GLuint shader, float r, float g, float b);
bool checkCollision(GameObject a, GameObject b);

int main() {
    // GLFW initialization
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Platformer", NULL, NULL);
    glfwMakeContextCurrent(window);
    glewInit();
/*
    // Configure VAO/VBO
    GLuint VAO, VBO;
    float vertices[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f,  0.5f
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Shader setup
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);


    // Projection matrix (2D orthographic)
    float projection[8][8] = (-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
*/
    // Game loop
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        

        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
/*
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
*/
    glfwTerminate();
    return 0;
}

// Input handling
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    player.velocityX = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        player.velocityX = -1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        player.velocityX = 1.0f;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && player.onGround)
        player.velocityY = 2.0f;
}

/*
// Game logic
void updateGame(float deltaTime) {
    // Apply gravity
    player.velocityY -= 9.8f * deltaTime;
    
    // Update position
    player.x += player.velocityX * deltaTime;
    player.y += player.velocityY * deltaTime;


    // Platform collision
   if (checkCollision(player, platform)) {
        player.y = platform.y + platform.height/2 + player.height/2;
        player.velocityY = 0.0f;
        player.onGround = true;
    } else {
        player.onGround = false;
    }
    
}

// Rendering
void renderObject(GameObject obj, GLuint shader, float r, float g, float b) {
    glUseProgram(shader);
    glUniform3f(glGetUniformLocation(shader, "color"), r, g, b);
    
    float model[16] = (1.0f);
    model = glm::translate(model, glm::vec3(obj.x, obj.y, 0.0f));
    model = glm::scale(model, glm::vec3(obj.width, obj.height, 1.0f));
    
    GLint modelLoc = glGetUniformLocation(shader, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// AABB collision detection
bool checkCollision(GameObject a, GameObject b) {
    return (a.x - a.width/2 < b.x + b.width/2 &&
            a.x + a.width/2 > b.x - b.width/2 &&
            a.y - a.height/2 < b.y + b.height/2 &&
            a.y + a.height/2 > b.y - b.height/2);
}
*/
