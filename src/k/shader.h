#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    GLuint id;
} Shader;

static char* load_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "ERROR::SHADER::FAILED_TO_OPEN_FILE: %s\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(file);
        fprintf(stderr, "ERROR::SHADER::MEMORY_ALLOCATION_FAILED\n");
        return NULL;
    }

    size_t readSize = fread(buffer, 1, size, file);
    buffer[readSize] = '\0';
    fclose(file);
    return buffer;
}

static void check_shader_errors(GLuint shader, const char* type) {
    GLint success;
    char infoLog[1024];
    if (strcmp(type, "PROGRAM") == 0) {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            fprintf(stderr, "ERROR::PROGRAM_LINKING_ERROR\n%s\n", infoLog);
        }
    } else {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            fprintf(stderr, "ERROR::%s_SHADER_COMPILATION_ERROR\n%s\n", type, infoLog);
        }
    }
}

static Shader shader_create(const char* vertex_path, const char* fragment_path) {
    Shader shader = {0};

    char* vertex_code = load_file(vertex_path);
    char* fragment_code = load_file(fragment_path);
    if (!vertex_code || !fragment_code) {
        free(vertex_code);
        free(fragment_code);
        return shader;
    }

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, (const GLchar**)&vertex_code, NULL);
    glCompileShader(vertex);
    check_shader_errors(vertex, "VERTEX");

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, (const GLchar**)&fragment_code, NULL);
    glCompileShader(fragment);
    check_shader_errors(fragment, "FRAGMENT");

    shader.id = glCreateProgram();
    glAttachShader(shader.id, vertex);
    glAttachShader(shader.id, fragment);
    glLinkProgram(shader.id);
    check_shader_errors(shader.id, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    free(vertex_code);
    free(fragment_code);

    return shader;
}

static void shader_use(const Shader* shader) {
    glUseProgram(shader->id);
}

static void shader_set_bool(const Shader* shader, const char* name, int value) {
    glUniform1i(glGetUniformLocation(shader->id, name), value);
}

static void shader_set_int(const Shader* shader, const char* name, int value) {
    glUniform1i(glGetUniformLocation(shader->id, name), value);
}

static void shader_set_float(const Shader* shader, const char* name, float value) {
    glUniform1f(glGetUniformLocation(shader->id, name), value);
}

#endif

