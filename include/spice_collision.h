#ifndef __SPICE_COLLISION_H__
#define __SPICE_COLLISION_H__
#include <spice_util.h>
#include <spice_math.h>
#include <spice_shape.h>

sp_status spicePolyCollideSAT(sp_convex_polygon* a, sp_convex_polygon* b);
sp_status spicePolyRectCollide(sp_convex_polygon* a, sp_rect* b);
sp_status spiceAABBCollide(sp_rect* r1, sp_rect* r2);

sp_status spiceBoundingBoxCollide(tm_vec3* a, tm_vec3* b);
sp_status spiceBoundingBoxCollideRay(tm_vec3* a, tm_vec3* b);
sp_status spiceSphereCollideRay(tm_vec3 rayOrigin, tm_vec3 rayDir, tm_vec3 sphereCenter, float sphereRadius);

sp_convex_polygon* spiceNewPolyConvex(size_t point_count);
sp_convex_polygon* spiceInitPolyConvex(size_t point_count, const sp_vec2* points);
void spiceFreePolyConvex(sp_convex_polygon* poly);

void spicePolyAddPoint(sp_convex_polygon* polygon, sp_vec2 point);

#endif