#include <spice_shape.h>
#include <spice_graphics.h>

// Draws outline of or filled sp_convex_polygon polygon to the screen with given color
void spiceGraphicsDrawPolygon(sp_convex_polygon* polygon, sp_vec4 color, uint8_t filled){
    if(filled) {
        GPU_PolygonFilled(spiceGraphicsWindowTarget(), polygon->point_count, (float*)polygon->points, (SDL_Color){color.r, color.g, color.b, color.a});
    } else {
        GPU_Polygon(spiceGraphicsWindowTarget(), polygon->point_count, (float*)polygon->points, (SDL_Color){color.r, color.g, color.b, color.a});
    }
}

// Draws outline of or filled sp_rect rect to the screen with given color
void spiceGraphicsDrawRect(sp_rect* rect, sp_vec4 color, uint8_t filled){
    if(filled){
        GPU_RectangleFilled(spiceGraphicsWindowTarget(), rect->position.x, rect->position.y, rect->position.x + rect->size.x, rect->position.y + rect->position.y, (SDL_Color){color.r, color.g, color.b, color.a});
    } else {
        GPU_Rectangle(spiceGraphicsWindowTarget(), rect->position.x, rect->position.y, rect->position.x + rect->size.x, rect->position.y + rect->position.y, (SDL_Color){color.r, color.g, color.b, color.a});
    }
}