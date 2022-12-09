#ifndef __SPICE_COLLISION_H__
#define __SPICE_COLLISION_H__
#include <spice_util.h>
#include <spice_math.h>

typedef struct POLY_CONVEX_S {
    size_t point_count;
    sp_vec2* points;
    uint8_t colliding;
} sp_convex_polygon;

int spicePolyCollideSAT(sp_convex_polygon* a, sp_convex_polygon* b);

sp_convex_polygon* spiceNewPolyConvex(size_t point_count);
sp_convex_polygon* spiceInitPolyConvex(size_t point_count, const sp_vec2* points);
void spiceFreePolyConvex(sp_convex_polygon* poly);

void spicePolyAddPoint(sp_convex_polygon* polygon, sp_vec2 point);

#endif