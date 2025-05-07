#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <AL/al.h>
#include <AL/alc.h>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include <iostream>

// Constants
const int WIDTH = 1280;
const int HEIGHT = 720;
const float PADDLE_SPEED = 800.0f;
const float INITIAL_BALL_SPEED = 400.0f;
const float BALL_ACCELERATION = 1.05f;
const float MAX_BALL_SPEED = 1200.0f;

// Game state
struct GameState {
    float leftPaddleY = HEIGHT / 2.0f;
    float rightPaddleY = HEIGHT / 2.0f;
    float ballX = WIDTH / 2.0f;
    float ballY = HEIGHT / 2.0f;
    float ballVelX = INITIAL_BALL_SPEED;
    float ballVelY = 0.0f;
    int leftScore = 0;
    int rightScore = 0;
    bool paused = true;
    float timeSinceHit = 0.0f;
};

GameState gameState;

// Particle system
struct Particle {
    float x, y;
    float velX, velY;
    float life;
    float r, g, b;
};

std::vector<Particle> particles;

// Audio system
ALCdevice* alDevice = nullptr;
ALCcontext* alContext = nullptr;
ALuint paddleSound, wallSound, scoreSound;

// Color theme
struct ColorTheme {
    float bg[3];
    float paddle[3];
    float ball[3];
    float particle1[3];
    float particle2[3];
};

ColorTheme currentTheme;
std::vector<ColorTheme> themes = {
    {{0.1f, 0.1f, 0.2f}, {0.8f, 0.2f, 0.2f}, {0.9f, 0.9f, 0.2f}, {0.9f, 0.5f, 0.1f}, {0.9f, 0.9f, 0.1f}},
    {{0.2f, 0.1f, 0.1f}, {0.2f, 0.8f, 0.2f}, {0.2f, 0.9f, 0.9f}, {0.1f, 0.9f, 0.5f}, {0.1f, 0.9f, 0.9f}},
    {{0.1f, 0.2f, 0.1f}, {0.2f, 0.2f, 0.8f}, {0.9f, 0.2f, 0.9f}, {0.5f, 0.1f, 0.9f}, {0.9f, 0.1f, 0.9f}}
};
int currentThemeIndex = 0;

// Shader sources
const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec2 position;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * vec4(position, 0.0, 1.0);
    }
)glsl";

const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 color;
    void main() {
        FragColor = vec4(color, 1.0);
    }
)glsl";

const char* particleFragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 color;
    uniform float alpha;
    void main() {
        FragColor = vec4(color, alpha);
    }
)glsl";

GLuint shaderProgram, particleShaderProgram;

void initOpenGL() {
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        exit(EXIT_FAILURE);
    }

    // Compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLuint particleFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(particleFragmentShader, 1, &particleFragmentShaderSource, NULL);
    glCompileShader(particleFragmentShader);

    particleShaderProgram = glCreateProgram();
    glAttachShader(particleShaderProgram, vertexShader);
    glAttachShader(particleShaderProgram, particleFragmentShader);
    glLinkProgram(particleShaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(particleFragmentShader);

    // Set up projection matrix
    glUseProgram(shaderProgram);
    float ortho[16] = {
        2.0f/WIDTH, 0.0f, 0.0f, -1.0f,
        0.0f, -2.0f/HEIGHT, 0.0f, 1.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, ortho);

    glUseProgram(particleShaderProgram);
    projLoc = glGetUniformLocation(particleShaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, ortho);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void generateSound(ALuint& buffer, float freq, float duration, int type) {
    const int SAMPLE_RATE = 44100;
    int samples = static_cast<int>(duration * SAMPLE_RATE);
    short* data = new short[samples];
    
    for (int i = 0; i < samples; i++) {
        float t = i / static_cast<float>(SAMPLE_RATE);
        float value = 0.0f;
        
        switch (type) {
            case 0: // Square wave for paddle
                value = (fmod(t * freq, 1.0f) < 0.5f) ? 0.5f : -0.5f;
                break;
            case 1: // Sine wave for wall
                value = 0.5f * sin(2 * M_PI * freq * t);
                break;
            case 2: // Decaying sine for score
                value = 0.7f * sin(2 * M_PI * freq * t) * (1.0f - t/duration);
                break;
        }
        
        data[i] = static_cast<short>(value * 32767);
    }
    
    alGenBuffers(1, &buffer);
    alBufferData(buffer, AL_FORMAT_MONO16, data, samples * sizeof(short), SAMPLE_RATE);
    
    delete[] data;
}

void initAudio() {
    alDevice = alcOpenDevice(nullptr);
    if (!alDevice) {
        std::cerr << "Failed to open OpenAL device" << std::endl;
        return;
    }

    alContext = alcCreateContext(alDevice, nullptr);
    if (!alContext) {
        std::cerr << "Failed to create OpenAL context" << std::endl;
        alcCloseDevice(alDevice);
        return;
    }
    alcMakeContextCurrent(alContext);

    // Generate sounds programmatically
    generateSound(paddleSound, 440.0f, 0.1f, 0);  // Short square wave
    generateSound(wallSound, 880.0f, 0.15f, 1);   // Sine wave
    generateSound(scoreSound, 220.0f, 1.0f, 2);   // Decaying sine
}

void cleanupAudio() {
    alDeleteBuffers(1, &paddleSound);
    alDeleteBuffers(1, &wallSound);
    alDeleteBuffers(1, &scoreSound);

    alcMakeContextCurrent(nullptr);
    if (alContext) alcDestroyContext(alContext);
    if (alDevice) alcCloseDevice(alDevice);
}

void playSound(ALuint buffer) {
    ALuint source;
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);
    alSourcePlay(source);
    
    // Set up deletion after playing
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING) {
        // In a real game, you'd want to manage sources properly
        alDeleteSources(1, &source);
    }
}

void createParticles(float x, float y, int count, const float* color1, const float* color2) {
    for (int i = 0; i < count; i++) {
        Particle p;
        p.x = x;
        p.y = y;
        p.velX = (rand() % 200 - 100) / 10.0f;
        p.velY = (rand() % 200 - 100) / 10.0f;
        p.life = 0.5f + (rand() % 100) / 100.0f;
        
        // Interpolate between two colors
        float t = (rand() % 100) / 100.0f;
        p.r = color1[0] * t + color2[0] * (1 - t);
        p.g = color1[1] * t + color2[1] * (1 - t);
        p.b = color1[2] * t + color2[2] * (1 - t);
        
        particles.push_back(p);
    }
}

void updateParticles(float deltaTime) {
    for (auto& p : particles) {
        p.x += p.velX * deltaTime;
        p.y += p.velY * deltaTime;
        p.life -= deltaTime;
    }
    
    particles.erase(std::remove_if(particles.begin(), particles.end(), 
        [](const Particle& p) { return p.life <= 0.0f; }), particles.end());
}

void resetBall(bool serveToRight) {
    gameState.ballX = WIDTH / 2.0f;
    gameState.ballY = HEIGHT / 2.0f;
    gameState.ballVelX = serveToRight ? INITIAL_BALL_SPEED : -INITIAL_BALL_SPEED;
    gameState.ballVelY = (rand() % 200 - 100) / 100.0f * INITIAL_BALL_SPEED * 0.5f;
    gameState.paused = true;
    gameState.timeSinceHit = 0.0f;
}

void updateGame(float deltaTime) {
    if (gameState.paused) {
        if (glfwGetTime() > 1.0f) { // Wait 1 second before serving
            gameState.paused = false;
        }
        return;
    }
    
    gameState.timeSinceHit += deltaTime;
    
    // Update ball position
    gameState.ballX += gameState.ballVelX * deltaTime;
    gameState.ballY += gameState.ballVelY * deltaTime;
    
    // Ball collision with top and bottom
    if (gameState.ballY <= 0 || gameState.ballY >= HEIGHT) {
        gameState.ballVelY = -gameState.ballVelY;
        gameState.ballY = gameState.ballY <= 0 ? 0 : HEIGHT;
        playSound(wallSound);
        createParticles(gameState.ballX, gameState.ballY, 20, currentTheme.particle1, currentTheme.particle2);
    }
    
    // Ball collision with paddles
    bool hit = false;
    
    // Left paddle
    if (gameState.ballX <= 30 && gameState.ballX >= 20 && 
        gameState.ballY >= gameState.leftPaddleY - 60 && gameState.ballY <= gameState.leftPaddleY + 60) {
        float hitPos = (gameState.ballY - gameState.leftPaddleY) / 60.0f;
        gameState.ballVelX = fabs(gameState.ballVelX) * BALL_ACCELERATION;
        gameState.ballVelY = hitPos * PADDLE_SPEED * 0.8f;
        gameState.ballX = 30;
        hit = true;
    }
    
    // Right paddle
    if (gameState.ballX >= WIDTH - 30 && gameState.ballX <= WIDTH - 20 && 
        gameState.ballY >= gameState.rightPaddleY - 60 && gameState.ballY <= gameState.rightPaddleY + 60) {
        float hitPos = (gameState.ballY - gameState.rightPaddleY) / 60.0f;
        gameState.ballVelX = -fabs(gameState.ballVelX) * BALL_ACCELERATION;
        gameState.ballVelY = hitPos * PADDLE_SPEED * 0.8f;
        gameState.ballX = WIDTH - 30;
        hit = true;
    }
    
    if (hit) {
        playSound(paddleSound);
        createParticles(gameState.ballX, gameState.ballY, 30, currentTheme.particle1, currentTheme.particle2);
        gameState.timeSinceHit = 0.0f;
        
        // Limit ball speed
        float speed = sqrt(gameState.ballVelX * gameState.ballVelX + gameState.ballVelY * gameState.ballVelY);
        if (speed > MAX_BALL_SPEED) {
            gameState.ballVelX = gameState.ballVelX / speed * MAX_BALL_SPEED;
            gameState.ballVelY = gameState.ballVelY / speed * MAX_BALL_SPEED;
        }
    }
    
    // Ball out of bounds
    if (gameState.ballX < 0) {
        gameState.rightScore++;
        playSound(scoreSound);
        createParticles(WIDTH / 4, HEIGHT / 2, 100, currentTheme.particle1, currentTheme.particle2);
        resetBall(true);
    } else if (gameState.ballX > WIDTH) {
        gameState.leftScore++;
        playSound(scoreSound);
        createParticles(3 * WIDTH / 4, HEIGHT / 2, 100, currentTheme.particle1, currentTheme.particle2);
        resetBall(false);
    }
}

void render() {
    glClearColor(currentTheme.bg[0], currentTheme.bg[1], currentTheme.bg[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Draw center line
    glUseProgram(shaderProgram);
    GLfloat lineVertices[] = {
        WIDTH / 2.0f, 0.0f,
        WIDTH / 2.0f, HEIGHT
    };
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    
    float lineColor[] = {0.3f, 0.3f, 0.4f};
    glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, lineColor);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, 2);
    
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    
    // Draw paddles
    GLfloat leftPaddleVertices[] = {
        20.0f, gameState.leftPaddleY - 60.0f,
        30.0f, gameState.leftPaddleY - 60.0f,
        30.0f, gameState.leftPaddleY + 60.0f,
        20.0f, gameState.leftPaddleY + 60.0f
    };
    
    GLfloat rightPaddleVertices[] = {
        WIDTH - 30.0f, gameState.rightPaddleY - 60.0f,
        WIDTH - 20.0f, gameState.rightPaddleY - 60.0f,
        WIDTH - 20.0f, gameState.rightPaddleY + 60.0f,
        WIDTH - 30.0f, gameState.rightPaddleY + 60.0f
    };
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(leftPaddleVertices), leftPaddleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    
    glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, currentTheme.paddle);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(rightPaddleVertices), rightPaddleVertices, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    
    // Draw ball
    const int SEGMENTS = 32;
    GLfloat ballVertices[SEGMENTS * 2 + 4];
    float radius = 10.0f + 5.0f * sin(gameState.timeSinceHit * 10.0f);
    
    ballVertices[0] = gameState.ballX;
    ballVertices[1] = gameState.ballY;
    for (int i = 0; i <= SEGMENTS; i++) {
        float angle = 2.0f * M_PI * i / SEGMENTS;
        ballVertices[2 + i * 2] = gameState.ballX + radius * cos(angle);
        ballVertices[3 + i * 2] = gameState.ballY + radius * sin(angle);
    }
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ballVertices), ballVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    
    glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, currentTheme.ball);
    glDrawArrays(GL_TRIANGLE_FAN, 0, SEGMENTS + 2);
    
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    
    // Draw particles
    if (!particles.empty()) {
        glUseProgram(particleShaderProgram);
        
        GLfloat particleVertices[particles.size() * 2];
        for (size_t i = 0; i < particles.size(); i++) {
            particleVertices[i * 2] = particles[i].x;
            particleVertices[i * 2 + 1] = particles[i].y;
        }
        
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(particleVertices), particleVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);
        
        glPointSize(3.0f);
        for (size_t i = 0; i < particles.size(); i++) {
            float alpha = particles[i].life * 2.0f;
            glUniform3f(glGetUniformLocation(particleShaderProgram, "color"), 
                       particles[i].r, particles[i].g, particles[i].b);
            glUniform1f(glGetUniformLocation(particleShaderProgram, "alpha"), alpha);
            glDrawArrays(GL_POINTS, i, 1);
        }
        
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        if (gameState.paused && gameState.timeSinceHit > 0.5f) {
            gameState.paused = false;
        }
    }
    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        currentThemeIndex = (currentThemeIndex + 1) % themes.size();
        currentTheme = themes[currentThemeIndex];
    }
}

int main() {
    srand(time(NULL));
    
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ultra Pong", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    
    initOpenGL();
    initAudio();
    
    currentTheme = themes[currentThemeIndex];
    resetBall(true);
    
    double lastTime = glfwGetTime();
    
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        // Handle input
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            gameState.leftPaddleY -= PADDLE_SPEED * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            gameState.leftPaddleY += PADDLE_SPEED * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            gameState.rightPaddleY -= PADDLE_SPEED * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            gameState.rightPaddleY += PADDLE_SPEED * deltaTime;
        }
        
        // Keep paddles in bounds
        gameState.leftPaddleY = std::max(60.0f, std::min((float)HEIGHT - 60.0f, gameState.leftPaddleY));
        gameState.rightPaddleY = std::max(60.0f, std::min((float)HEIGHT - 60.0f, gameState.rightPaddleY));
        
        updateGame(deltaTime);
        updateParticles(deltaTime);
        render();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    cleanupAudio();
    glfwTerminate();
    return 0;
}
