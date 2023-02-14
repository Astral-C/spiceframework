#ifndef __TURMERIC_MATRIX_H__
#define __TURMERIC_MATRIX_H__
#include "tm_vector.h"

#define tm_mat4_get(m, x, y) m[(x << 2) + y];
#define tm_mat4_set(m, x, y, v) m[(x << 2) + y] = v;

typedef float tm_mat4[16];

void tm_mat4_identity(tm_mat4 out);
void tm_perspective(float fov, float aspect, float near, float far, tm_mat4 out);
void tm_ortho(float left, float right, float top, float bottom, float near, float far, tm_mat4 out);
void tm_lookat(tm_vec3 position, tm_vec3 target, tm_vec3 up, tm_mat4 out);
void tm_mat4_mult_vec4(tm_mat4 mat, tm_vec4 vec, tm_vec4* out);
void tm_mat4_mult(tm_mat4 a, tm_mat4 b, tm_mat4 out);

#endif