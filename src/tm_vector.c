#include "tm_vector.h"

//////
///
/// Vector 2 Math Functions
///
//////

void vec2_add(tm_vec2 v1, tm_vec2 v2, tm_vec2* dest){
    dest->x = v1.x + v2.x;
    dest->y = v1.y + v2.y;
}

void vec2_sub(tm_vec2 v1, tm_vec2 v2, tm_vec2* dest){
    dest->x = v1.x - v2.x;
    dest->y = v1.y - v2.y;
}

void vec2_mul(tm_vec2 v1, tm_vec2 v2, tm_vec2* dest){
    dest->x = v1.x * v2.x;
    dest->y = v1.y * v2.y;
}

float vec2_dot(tm_vec2 v1, tm_vec2 v2){
    return (v1.x * v2.x) + (v1.y * v2.y);
}

//////
///
/// Vector 3 Math Functions
///
//////

void vec3_add(tm_vec3 v1, tm_vec3 v2, tm_vec3* dest){
    dest->x = v1.x + v2.x;
    dest->y = v1.y + v2.y;
    dest->z = v1.z + v2.z;
}

void vec3_sub(tm_vec3 v1, tm_vec3 v2, tm_vec3* dest){
    dest->x = v1.x - v2.x;
    dest->y = v1.y - v2.y;
    dest->z = v1.z - v2.z;
}

void vec3_mul(tm_vec3 v1, tm_vec3 v2, tm_vec3* dest){
    dest->x = v1.x * v2.x;
    dest->y = v1.y * v2.y;
    dest->z = v1.z * v2.z;
}

float vec3_dot(tm_vec3 v1, tm_vec3 v2){
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

void vec3_cross(tm_vec3 v1, tm_vec3 v2, tm_vec3* out){
    out->x = v1.y * v2.z - v1.z * v1.y;
    out->y = v1.z * v2.x - v1.x * v1.z;
    out->z = v1.x * v2.y - v1.y * v1.x;
}

void vec3_norm(tm_vec3* v){
    float w = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);

    if(w == 0.0){
        v->x = v->y = v->z = 0.0f;
        return;
    }

    v->x /= w;
    v->y /= w;
    v->z /= w;
}

//////
///
/// Vector 4 Math Functions
///
//////

void vec4_add(tm_vec4 v1, tm_vec4 v2, tm_vec4* dest){
    dest->x = v1.x + v2.x;
    dest->y = v1.y + v2.y;
    dest->z = v1.z + v2.z;
    dest->w = v1.w + v2.w;
}

void vec4_sub(tm_vec4 v1, tm_vec4 v2, tm_vec4* dest){
    dest->x = v1.x - v2.x;
    dest->y = v1.y - v2.y;
    dest->z = v1.z - v2.z;
    dest->w = v1.w - v2.w;
}

void vec4_mul(tm_vec4 v1, tm_vec4 v2, tm_vec4* dest){
    dest->x = v1.x * v2.x;
    dest->y = v1.y * v2.y;
    dest->z = v1.z * v2.z;
    dest->w = v1.w * v2.w;
}

float vec4_dot(tm_vec4 v1, tm_vec4 v2){
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) + (v1.w * v2.w);
}