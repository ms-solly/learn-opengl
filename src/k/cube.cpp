#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DEG2RAD(angle) ((angle) * M_PI / 180.0f)

// Basic 4x4 matrix (row-major) helper
void identity(float* mat) {
    for (int i = 0; i < 16; ++i) mat[i] = 0.0f;
    mat[0] = mat[5] = mat[10] = mat[15] = 1.0f;
}

void perspective(float* mat, float fov, float aspect, float nearZ, float farZ) {
    float tanHalfFov = tanf(DEG2RAD(fov) / 2.0f);
    identity(mat);
    mat[0] = 1.0f / (aspect * tanHalfFov);
    mat[5] = 1.0f / tanHalfFov;
    mat[10] = -(farZ + nearZ) / (farZ - nearZ);
    mat[11] = -1.0f;
    mat[14] = -(2.0f * farZ * nearZ) / (farZ - nearZ);
    mat[15] = 0.0f;
}

void translate(float* mat, float x, float y, float z) {
    identity(mat);
    mat[12] = x;
    mat[13] = y;
    mat[14] = z;
}

void rotateY(float* mat, float angle) {
    identity(mat);
    float c = cosf(angle);
    float s = sinf(angle);
    mat[0] = c;
    mat[2] = s;
    mat[8] = -s;
    mat[10] = c;
}

void multiply(float* res, float* a, float* b) {
    float tmp[16];
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col) {
            tmp[col + row * 4] = 0;
            for (int i = 0; i < 4; ++i)
                tmp[col + row * 4] += a[i + row * 4] * b[col + i * 4];
        }
    for (int i = 0; i < 16; ++i) res[i] = tmp[i];
}

// Vertex and fragment shaders
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 uMVP;\n"
"void main() {\n"
"   gl_Position = uMVP * vec4(aPos, 1.0);\n"
"}\n";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main() {\n"
"   FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
"}\n";

// Cube vertices
float cubeVertices[] = {
    // Front
    -0.5f, -0.5f,  0.5f, 
     0.5f, -0.5f,  0.5f, 
     0.5f,  0.5f,  0.5f, 
    -0.5f,  0.5f,  0.5f,

    // Back
    -0.5f, -0.5f, -0.5f, 
     0.5f, -0.5f, -0.5f, 
     0.5f,  0.5f, -0.5f, 
    -0.5f,  0.5f, -0.5f
};

// Index buffer (12 triangles = 6 faces)
unsigned int indices[] = {
    0, 1, 2, 2, 3, 0, // front
    4, 5, 6, 6, 7, 4, // back
    0, 4, 7, 7, 3, 0, // left
    1, 5, 6, 6, 2, 1, // right
    3, 2, 6, 6, 7, 3, // top
    0, 1, 5, 5, 4, 0  // bottom
};
int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    SDL_Window* window = SDL_CreateWindow("3D Cube - SDL + OpenGL",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
       800, 600, SDL_WINDOW_OPENGL);

    if(!window){
        fprintf(stderr,"w create failed");
    }
    SDL_GLContext context = SDL_GL_CreateContext(window);

    glewExperimental = GL_TRUE;
    glewInit();

    // Compile shaders
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vShader);

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vShader);
    glAttachShader(shaderProgram, fShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vShader);
    glDeleteShader(fShader);

    // Buffers
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);

    float proj[16], view[16], model[16], mvp[16], rot[16], mv[16];
    perspective(proj, 45.0f, 800.0f / 600.0f, 0.1f, 100.0f);
    translate(view, 0.0f, 0.0f, -3.0f);

    // Main loop
    int running = 1;
    SDL_Event event;
    float angle = 0.0f;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
        }

        angle += 0.01f;
        rotateY(rot, angle);
        multiply(mv, view, rot);
        multiply(mvp, proj, mv);

        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMVP"), 1, GL_FALSE, mvp);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

