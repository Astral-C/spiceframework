#ifndef __TURMERIC_VECTOR_H__
#define __TURMERIC_VECTOR_H__

#include <math.h>

#define lerpf(a, b, t) a + t * (b - a);

#define lerpv2(d, a, b, t) d.x = lerpf(a.x, b.x, t); d.y = lerpf(a.x, b.y, t);
#define lerpv3(d, a, b, t) d.x = lerpf(a.x, b.x, t); d.y = lerpf(a.x, b.y, t); d.z = lerpf(a.z, b.z, t);

typedef union {
    struct {
        float x, y;
    };
    struct {
        float w, h;
    };
} tm_vec2;

typedef struct {
    float x, y, z;
} tm_vec3;

typedef union {
    struct {
        float x, y, z, w;
    };
    struct {
        float r, g, b, a;
    };
} tm_vec4;



void vec2_add(tm_vec2 v1, tm_vec2 v2, tm_vec2* dest);

void vec2_sub(tm_vec2 v1, tm_vec2 v2, tm_vec2* dest);

void vec2_mul(tm_vec2 v1, tm_vec2 v2, tm_vec2* dest);

float vec2_dot(tm_vec2 v1, tm_vec2 v2);



void vec3_add(tm_vec3 v1, tm_vec3 v2, tm_vec3* dest);

void vec3_sub(tm_vec3 v1, tm_vec3 v2, tm_vec3* dest);

void vec3_mul(tm_vec3 v1, tm_vec3 v2, tm_vec3* dest);

void vec3_cross(tm_vec3 v1, tm_vec3 v2, tm_vec3* out);

float vec3_dot(tm_vec3 v1, tm_vec3 v2);

void vec3_norm(tm_vec3* v);



void vec4_add(tm_vec4 v1, tm_vec4 v2, tm_vec4* dest);

void vec4_sub(tm_vec4 v1, tm_vec4 v2, tm_vec4* dest);

void vec4_mul(tm_vec4 v1, tm_vec4 v2, tm_vec4* dest);

float vec4_dot(tm_vec4 v1, tm_vec4 v2);

#endif