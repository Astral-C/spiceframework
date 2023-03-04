#ifndef __SPICE_SHAPE_H__
#define __SPICE_SHAPE_H__
#include <spice_util.h>
#include <spice_math.h>

typedef struct POLY_CONVEX_S {
    size_t point_count;
    sp_vec2* points;
    uint8_t colliding;
} sp_convex_polygon;

typedef struct SPICE_RECT_S {
    sp_vec2 position;
    sp_vec2 size;
} sp_rect;

typedef struct {
    sp_vec3 min, max;
} sp_bounding_box;

void spiceGraphicsDrawPolygon(sp_convex_polygon* polygon, sp_vec4 color, uint8_t filled);
void spiceGraphicsDrawRect(sp_rect* rect, sp_vec4 color, uint8_t filled);

#endif