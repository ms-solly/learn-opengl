#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include<assert.h>
#include<string.h>
#include <math.h>


int initialize();
int init_shaders();
int init_geometry();
int init_textures();
int update();
int cleanup();

SDL_Window    *window = NULL;
SDL_GLContext context;
unsigned int  vao, vbo, ebo, tex;
//unsigned int  vert_shader,frag_shader;
unsigned int shader_prog;


void load_identity(float *m){

    memset(m, 0, sizeof(float) * 16);
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void perspective(float *m, float fovy, float aspect, float nearZ, float farZ){

    float f = 1.0f / tanf(fovy * 0.5f);
    load_identity(m);
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (farZ + nearZ) / (nearZ - farZ);
    m[11] = -1.0f;
    m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
    m[15] = 0.0f;
}

void rotate_Y(float *m, float angle){

    load_identity(m);
    float c = cosf(angle), s = sinf(angle);
    m[0] = c;
    m[2] = s;
    m[5] = 1.0f;
    m[8] = -s;
    m[10] = c;
    m[15] = 1.0f;
}
void multiply_matrices(float *result, const float *a, const float *b) {

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += a[row * 4 + k] * b[k * 4 + col];
            }
            result[row * 4 + col] = sum;
        }
    }
}

void translate(float *m, float tx, float ty, float tz) {

load_identity(m);

m[12] = tx;

m[13] = ty;

m[14] = tz;

}



const GLfloat verts[] = {
};

const GLuint indices[] = {

};

char *read_file(const char* filepath){

    FILE* file = fopen(filepath, "rb");
    assert(file && "failed to access shader file");
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* sh_buffer = (char*)malloc(length + 1);
    assert(sh_buffer !=NULL && "Memory allocation failed");
    size_t rc = fread(sh_buffer, 1, length, file);
    assert(rc == (size_t)length && "Incomplete file read!");
    fclose(file);

    sh_buffer[length] = '\0'; 

    return sh_buffer;
}



static unsigned int compile_shader(unsigned int type, const char* source){

    unsigned  int id = glCreateShader(type);
    assert(id != 0 && "Failed to create shader");
    const char* src  = &source[0];
    glShaderSource(id, 1, &src,NULL);
    glCompileShader(id);

    //Error handling
    int result ;
    glGetShaderiv(id, GL_COMPILE_STATUS,&result);

    if(result != GL_TRUE) {
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

static unsigned int create_shader(const char* vertex_shader, const char* fragment_shader){

    unsigned int prog = glCreateProgram();
    assert(prog && "Failed to create shader program");

    printf("Vertex Shader:\n%s\n", vertex_shader);
    printf("Fragment Shader:\n%s\n", fragment_shader);


    unsigned int vs   = compile_shader(GL_VERTEX_SHADER,vertex_shader);
    assert(vs && "vertex shader compilation failed.");
    
    unsigned int fs   = compile_shader(GL_FRAGMENT_SHADER,fragment_shader);
    assert(fs && "fragment shader compilation failed.");

    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glValidateProgram(prog);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return prog;

}

/*
 * Basic Initialization
 */
int initialize(){

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
    if (window == NULL){
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

    init_shaders();
    init_geometry();
   //  init_textures();

    return 0;
}

/*
 * Initialize Shaders
 */
int init_shaders(){

    char* vert_src = read_file("res/shaders/vertex.glsl");
    char* frag_src = read_file("res/shaders/fragment.glsl");

    if (!vert_src || !frag_src) {
        fprintf(stderr, "failed shader loadig (init_shader)\n");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, 
                                 "Shader Error", 
                                 "Failed to load shader files", 
                                 window);
    exit(1);
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // create & Compile vertex shader
    shader_prog = create_shader(vert_src, frag_src);
    if (!shader_prog) {
        fprintf(stderr, "Shader program creation failed.\n");
        return 1;
    }
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL error: %d\n", err);
    }
    glUseProgram(shader_prog);
    free(vert_src);
    free(frag_src);

    return 0;
}

/*
 * Initialize Geometry
 */
int init_geometry(){
    // Populate vertex buffer
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // Populate element buffer
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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

int init_textures(){

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
int cleanup(){

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

int update(){

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glBindVertexArray(vao);
    glUseProgram(shader_prog);

    glDrawArrays(GL_POINTS, 0, 1);
    SDL_GL_SwapWindow(window);
    return 0;
}

/*
 * Program entry point
 */
int main(int argc, char *argv[]){

    int should_run;

    printf("Initializing...\n");
    if (initialize()) {
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

        update();
    }

    printf("Exiting...\n");
    cleanup();
    return 0;
}
