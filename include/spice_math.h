#ifndef __SPICE_MATH_H__
#define __SPICE_MATH_H__

#define vec2_add(v1, v2, dest) dest.x = v1.x + v2.x; dest.y = v1.y + v2.y;
#define vec2_sub(v1, v2, dest) dest.x = v1.x - v2.x; dest.y = v1.y - v2.y;
#define vec2_mul(v1, v2, dest) dest.x = v1.x * v2.x; dest.y = v1.y * v2.y;
#define vec2_dot(v1, v2) ((v1.x * v2.x) + (v1.y * v2.y))

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
} sp_vec2;

typedef struct {
    float x, y, z;
} sp_vec3;

typedef union {
    struct {
        float x, y, z, w;
    };
    struct {
        float r, g, b, a;
    };
} sp_vec4;


#endif