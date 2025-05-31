
#ifndef DEV_UTIL_H
#define DEV_UTIL_H

#ifndef _WIN64
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>
#include <limits.h>

#ifndef DEV_VULKAN
#include <GL/glew.h>
#endif

#include "types.h"  // assumes basic typedefs like 'uint' etc. are here

// Replace std::string with char*
int ReadTextFile(const char* fileName, char** outText);
char* ReadBinaryFile(const char* pFileName, int* outSize);
int WriteBinaryFile(const char* pFilename, const void* pData, int size);

// Logging and error handling
void Error(const char* pFileName, unsigned int line, const char* msg, ...);
void FileError(const char* pFileName, unsigned int line, const char* pFileError);

// Convenience macros
#define ERROR0(msg) OgldevError(__FILE__, __LINE__, msg)
#define ERROR(msg, ...) OgldevError(__FILE__, __LINE__, msg, __VA_ARGS__)
#define FILE_ERROR(FileError) OgldevFileError(__FILE__, __LINE__, FileError)

#define ZERO_MEM(a) memset(a, 0, sizeof(*(a)))
#define ZERO_MEM_VAR(var) memset(&(var), 0, sizeof((var)))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a) / sizeof((a)[0]))
#define ARRAY_SIZE_IN_BYTES(a, size) ((sizeof((a)[0])) * (size))

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifdef _WIN64
#define SNPRINTF _snprintf_s
#define VSNPRINTF vsnprintf_s
#define RANDOM rand
#define SRANDOM srand((unsigned)time(NULL))
#pragma warning(disable : 4566)
#else
#define SNPRINTF snprintf
#define VSNPRINTF vsnprintf
#define RANDOM random
#define SRANDOM srandom(getpid())
#endif

#define INVALID_UNIFORM_LOCATION 0xffffffff
#define INVALID_OGL_VALUE        0xffffffff
#define NUM_CUBE_MAP_FACES       6

#define SAFE_DELETE(p) if ((p) != NULL) { free(p); (p) = NULL; }

long long GetCurrentTimeMillis(void);

#define ASSIMP_LOAD_FLAGS (aiProcess_JoinIdenticalVertices |    \
                           aiProcess_Triangulate |              \
                           aiProcess_GenSmoothNormals |         \
                           aiProcess_LimitBoneWeights |         \
                           aiProcess_SplitLargeMeshes |         \
                           aiProcess_ImproveCacheLocality |     \
                           aiProcess_RemoveRedundantMaterials | \
                           aiProcess_FindDegenerates |          \
                           aiProcess_FindInvalidData |          \
                           aiProcess_GenUVCoords |              \
                           aiProcess_CalcTangentSpace)

#define NOT_IMPLEMENTED \
    do { printf("Not implemented: %s:%d\n", __FILE__, __LINE__); exit(0); } while (0)


#define GLExitIfError                                                        \
    do {                                                                     \
        GLenum Error = glGetError();                                         \
        if (Error != GL_NO_ERROR) {                                          \
            printf("OpenGL error in %s:%d: 0x%x\n", __FILE__, __LINE__, Error); \
            exit(1);                                                         \
        }                                                                    \
    } while (0)

#define GLCheckError() (glGetError() == GL_NO_ERROR)

void gl_check_error(const char* function, const char *file, int line);

#define CHECK_GL_ERRORS

#ifdef CHECK_GL_ERRORS
#define GCE gl_check_error(__FUNCTION__, __FILE__, __LINE__)
#else
#define GCE
#endif

void glDebugOutput(GLenum source,
                   GLenum type,
                   unsigned int id,
                   GLenum severity,
                   GLsizei length,
                   const char *message,
                   const void *userParam);
#endif

// Utilities
char* GetDirFromFilename(const char* Filename);  // caller should free the result

#define MAX_BONES (200)

#define CLAMP(Val, Start, End) (MIN(MAX((Val), (Start)), (End)))

int GetGLMajorVersion(void);
int GetGLMinorVersion(void);
int IsGLVersionHigher(int MajorVer, int MinorVer);

#endif  // OGLDEV_UTIL_H

