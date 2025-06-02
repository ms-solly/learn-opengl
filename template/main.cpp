#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include<assert.h>
#include<string.h>
#include <math.h>

void LoadIdentity(float *m) {
    memset(m, 0, sizeof(float) * 16);
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void Perspective(float *m, float fovy, float aspect, float nearZ, float farZ) {
    float f = 1.0f / tanf(fovy * 0.5f);
    LoadIdentity(m);
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (farZ + nearZ) / (nearZ - farZ);
    m[11] = -1.0f;
    m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
    m[15] = 0.0f;
}

void RotateY(float *m, float angle) {
    LoadIdentity(m);
    float c = cosf(angle), s = sinf(angle);
    m[0] = c;
    m[2] = s;
    m[5] = 1.0f;
    m[8] = -s;
    m[10] = c;
    m[15] = 1.0f;
}


SDL_Window    *window = NULL;
SDL_GLContext context;
unsigned int  vao, vbo, ebo, tex;
unsigned int  vert_shader,frag_shader, shader_prog;

const GLfloat cube_verts[] = {
    -0.5f, -0.5f, 0.5f,
     0.5f, -0.5f, 0.5f,
     0.5f,  0.5f, 0.5f,
    -0.5f,  0.5f, 0.5f,

    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,

    -0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,

    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f,  0.5f
};

const GLuint cube_indices[] = {
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
};

int Initialize();
int Update();
int Cleanup();
int InitShaders();
int InitGeometry();
int InitTextures();

static unsigned int Compile_Shader(unsigned int type, const char* source){
    unsigned  int id = glCreateShader(type);
    assert(id != 0 && "Failed to create shader");
    const char* src  = &source[0];
    glShaderSource(id, 1, &src,NULL);
    glCompileShader(id);


    //Error handling
    int result ;
    glGetShaderiv(id, GL_COMPILE_STATUS,&result);

    if(result != GL_TRUE){
        char err_buff[512];
        glGetShaderInfoLog(id, sizeof(err_buff), NULL, err_buff);
        err_buff[sizeof(err_buff) - 1] = '\0';

        const char* type_str = type == GL_VERTEX_SHADER ? "Vertex" :
                               type == GL_FRAGMENT_SHADER ? "Fragment" : "Unknown";

        fprintf(stderr, "%s shader compilation failed:\n%s\n", type_str, err_buff);

        glDeleteShader(id);

        return 0;
    }


    return id;

} 

static unsigned int Create_Shader(const char* vertex_shader, const char* fragment_shader){

    unsigned int prog = glCreateProgram();
    assert(prog && "Failed to create shader program");

    unsigned int vs   = Compile_Shader(GL_VERTEX_SHADER,vertex_shader);
    assert(vs && "vertex shader compilation failed.");
    
    unsigned int fs   = Compile_Shader(GL_FRAGMENT_SHADER,fragment_shader);
    assert(fs && "fragment shader compilation failed.");

    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glValidateProgram(prog);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return prog;

}

const char *vert_shader_src = "\
#version 150 core\n\
in vec3 in_Position;\n\
uniform mat4 u_MVP;\n\
void main() {\n\
    gl_Position = u_MVP * vec4(in_Position, 1.0);\n\
}\n\
";

const char *frag_shader_src = "\
#version 150 core\n\
out vec4 out_Color;\n\
void main() {\n\
    out_Color = vec4(0.0, 0.8, 1.0, 1.0);\n\
}\n\
";


/*
 * Basic Initialization
 */
int Initialize()
{
    // Initialize SDL Video
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL video\n");
        return 1;
    }

    // Create main window
    window = SDL_CreateWindow(
        "SDL App",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 480,
        SDL_WINDOW_OPENGL);
    if (window == NULL) {
        fprintf(stderr, "Failed to create main window\n");
        SDL_Quit();
        return 1;
    }

    // Initialize rendering context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        fprintf(stderr, "Failed to create GL context\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_SetSwapInterval(1); // Use VSYNC

    // Initialize GL Extension Wrangler (GLEW)
    GLenum err;
    glewExperimental = GL_TRUE; // Please expose OpenGL 3.x+ interfaces
    err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Failed to init GLEW\n");
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    InitShaders();
    InitGeometry();
   //  InitTextures();

    return 0;
}

/*
 * Initialize Shaders
 */
int InitShaders()
{
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // create & Compile vertex shader
    shader_prog = Create_Shader(vert_shader_src, frag_shader_src);
    if (!shader_prog) {
        fprintf(stderr, "Shader program creation failed.\n");
        return 1;
    }

    glUseProgram(shader_prog);
    
      return 0;
}

/*
 * Initialize Geometry
 */
int InitGeometry()
{
    // Populate vertex buffer
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_verts), cube_verts, GL_STATIC_DRAW);

    // Populate element buffer
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    // Bind vertex position attribute
    GLint pos_attr_loc = glGetAttribLocation(shader_prog, "in_Position");
    if (pos_attr_loc == -1) {
        fprintf(stderr, "Could not find 'in_Position' attribute\n");
        return 1;
    }    
    glVertexAttribPointer(pos_attr_loc, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(pos_attr_loc);

/*
    //  Bind vertex texture coordinate attribute
    GLint tex_attr_loc = glGetAttribLocation(shader_prog, "in_Texcoord");
    glVertexAttribPointer(tex_attr_loc, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
    glEnableVertexAttribArray(tex_attr_loc);
*/ 
    return 0;
}

/*
 * Initialize textures

int InitTextures()
{
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glUniform1i(glGetUniformLocation(shader_prog, "tex"), 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, logo_rgba);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return 0;
}
*/


/*
 * Free resources
 */
int Cleanup()
{
    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glDeleteProgram(shader_prog);
 // glDeleteTextures(1, &tex);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

/*
 * Render a frame
 */

int Update()
{
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao);
    glUseProgram(shader_prog);

    static float angle = 0.0f;
    angle += 0.01f;

    float proj[16], rot[16], mvp[16];
    Perspective(proj, 45.0f * M_PI / 180.0f, 640.0f/480.0f, 0.1f, 100.0f);
    RotateY(rot, angle);

    // Multiply MVP = proj * rot
    // Naive 4x4 matrix multiply
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            mvp[row * 4 + col] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                mvp[row * 4 + col] += proj[row * 4 + k] * rot[k * 4 + col];
            }
        }
    }

    GLint loc = glGetUniformLocation(shader_prog, "u_MVP");
    glUniformMatrix4fv(loc, 1, GL_FALSE, mvp);

    glDrawElements(GL_LINES, sizeof(cube_indices)/sizeof(GLuint), GL_UNSIGNED_INT, 0);

    SDL_GL_SwapWindow(window);
    return 0;
}

/*
 * Program entry point
 */
int main(int argc, char *argv[])
{
    int should_run;

    printf("Initializing...\n");
    if (Initialize()) {
        return 1;
    }

    printf("Running...\n");
    for (should_run = 1; should_run; ) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
                should_run = 0;
                break;
            }
        }

        Update();
    }

    printf("Exiting...\n");
    Cleanup();
    return 0;
}
