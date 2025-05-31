#include "math_3d.h"

Vector3f Vector3f_Add(Vector3f a, Vector3f b) {
    return (Vector3f){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vector3f Vector3f_Sub(Vector3f a, Vector3f b) {
    return (Vector3f){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vector3f Vector3f_Scale(Vector3f v, float s) {
    return (Vector3f){v.x * s, v.y * s, v.z * s};
}

float Vector3f_Dot(Vector3f a, Vector3f b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3f Vector3f_Cross(Vector3f a, Vector3f b) {
    return (Vector3f){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

float Vector3f_Length(Vector3f v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vector3f Vector3f_Normalize(Vector3f v) {
    float len = Vector3f_Length(v);
    if (len != 0.0f) {
        return Vector3f_Scale(v, 1.0f / len);
    }
    return v;
}

void Vector3f_Print(Vector3f v) {
    printf("(%f, %f, %f)\n", v.x, v.y, v.z);
}

void Matrix4f_Identity(Matrix4f* mat) {
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            mat->m[i][j] = (i == j) ? 1.0f : 0.0f;
}

Matrix4f Matrix4f_Mul(Matrix4f* a, Matrix4f* b) {
    Matrix4f result = {0};
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k)
                result.m[i][j] += a->m[i][k] * b->m[k][j];
    return result;
}

Vector4f Matrix4f_MulVector(Matrix4f* mat, Vector4f v) {
    Vector4f r = {0};
    r.x = mat->m[0][0] * v.x + mat->m[0][1] * v.y + mat->m[0][2] * v.z + mat->m[0][3] * v.w;
    r.y = mat->m[1][0] * v.x + mat->m[1][1] * v.y + mat->m[1][2] * v.z + mat->m[1][3] * v.w;
    r.z = mat->m[2][0] * v.x + mat->m[2][1] * v.y + mat->m[2][2] * v.z + mat->m[2][3] * v.w;
    r.w = mat->m[3][0] * v.x + mat->m[3][1] * v.y + mat->m[3][2] * v.z + mat->m[3][3] * v.w;
    return r;
}

void Matrix4f_Transpose(Matrix4f* result, Matrix4f* mat) {
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            result->m[i][j] = mat->m[j][i];
}

