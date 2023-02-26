#ifndef __SPICE_MATH_H__
#define __SPICE_MATH_H__

#include <tm_vector.h>

#define lerpf(a, b, t) a + t * (b - a);

#define lerpv2(d, a, b, t) d.x = lerpf(a.x, b.x, t); d.y = lerpf(a.x, b.y, t);
#define lerpv3(d, a, b, t) d.x = lerpf(a.x, b.x, t); d.y = lerpf(a.x, b.y, t); d.z = lerpf(a.z, b.z, t);

typedef tm_vec2 sp_vec2;

typedef tm_vec3 sp_vec3;

typedef tm_vec4 sp_vec4;


#endif