#ifndef MAT4_H
#define MAT4_H

#include <cstring>

struct Mat4 {
    float data[16];  // column-major OpenGL format
};

// Function to initialize a matrix as the identity matrix
inline void mat4_identity(Mat4& mat) {
    std::memset(mat.data, 0, sizeof(mat.data));
    mat.data[0] = mat.data[5] = mat.data[10] = mat.data[15] = 1.0f;
}

// Function to create an orthographic projection matrix
inline void mat4_orthographic(Mat4& mat, float left, float right, float bottom, float top, float nearZ, float farZ) {
    mat4_identity(mat);
    mat.data[0]  = 2.0f / (right - left);
    mat.data[5]  = 2.0f / (top - bottom);
    mat.data[10] = -2.0f / (farZ - nearZ);
    mat.data[12] = -(right + left) / (right - left);
    mat.data[13] = -(top + bottom) / (top - bottom);
    mat.data[14] = -(farZ + nearZ) / (farZ - nearZ);
}

// Function to create a translation matrix
inline void mat4_translate(Mat4& mat, float x, float y, float z) {
    mat4_identity(mat);
    mat.data[12] = x;
    mat.data[13] = y;
    mat.data[14] = z;
}

// Function to create a scaling matrix
inline void mat4_scale(Mat4& mat, float sx, float sy, float sz) {
    mat4_identity(mat);
    mat.data[0] = sx;
    mat.data[5] = sy;
    mat.data[10] = sz;
}

// Function to multiply two matrices
inline void mat4_multiply(const Mat4& a, const Mat4& b, Mat4& result) {
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            result.data[col + row * 4] = 0.0f;
            for (int i = 0; i < 4; ++i) {
                result.data[col + row * 4] += a.data[i + row * 4] * b.data[col + i * 4];
            }
        }
    }
}

#endif

