#ifndef MATH_3D_H
#define MATH_3D_H

#include <math.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct Vector3f {
    float x;
    float y;
    float z;
} Vector3f;

typedef struct Vector4f {
    float x;
    float y;
    float z;
    float w;
} Vector4f;

typedef struct Matrix4f {
    float m[4][4];
} Matrix4f;

// Vector3f functions
Vector3f Vector3f_Add(Vector3f a, Vector3f b);
Vector3f Vector3f_Sub(Vector3f a, Vector3f b);
Vector3f Vector3f_Scale(Vector3f v, float s);
float    Vector3f_Dot(Vector3f a, Vector3f b);
Vector3f Vector3f_Cross(Vector3f a, Vector3f b);
float    Vector3f_Length(Vector3f v);
Vector3f Vector3f_Normalize(Vector3f v);
void     Vector3f_Print(Vector3f v);

// Matrix4f functions
void     Matrix4f_Identity(Matrix4f* mat);
Matrix4f Matrix4f_Mul(Matrix4f* a, Matrix4f* b);
Vector4f Matrix4f_MulVector(Matrix4f* mat, Vector4f v);
void     Matrix4f_Transpose(Matrix4f* result, Matrix4f* mat);

#endif // MATH_3D_H

