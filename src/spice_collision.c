#include <SDL_gpu.h>
#include <spice_collision.h>

sp_convex_polygon* spNewPolyConvex(size_t point_count){
    sp_convex_polygon* poly = malloc(sizeof(sp_convex_polygon));
}

sp_convex_polygon* spInitPolyConvex(size_t point_count, const sp_vec2* points){
    sp_convex_polygon* poly = malloc(sizeof(sp_convex_polygon));
    poly->point_count = point_count;
    poly->points = malloc(sizeof(sp_vec2) * point_count);
    poly->colliding = 0;
    memcpy(poly->points, points, sizeof(sp_vec2) * point_count);
    return poly;
}

void spFreePolyConvex(sp_convex_polygon* poly){
    if(poly == NULL) return;
    if(poly->points != NULL){
        free(poly->points);
    }
    free(poly);
}

void spPolyAddPoint(sp_convex_polygon* polygon, sp_vec2 point){
    if(polygon->points){
        polygon->points = realloc(polygon->points, sizeof(sp_vec2) * polygon->point_count + 1);
    } else {
        polygon->points = malloc(sizeof(sp_vec2));
    }
    memcpy(&polygon->points[polygon->point_count], &point, sizeof(point));
    polygon->point_count++;
}

// Based on the function from OLC's video on SAT.
// if it ain't broke...
int spPolyCollideSAT(sp_convex_polygon* a, sp_convex_polygon* b){
    for (size_t ply = 0; ply < 2; ply++){
        if(ply == 1){
            sp_convex_polygon* t = a;
            a = b;
            b = t;
        }
        for (size_t p = 0; p < a->point_count; p++){
            sp_vec2* p1 = &a->points[p];
            sp_vec2* p2 = &a->points[(p + 1) % a->point_count];
            sp_vec2 axis = (sp_vec2){-(p2->y - p1->y), (p2->x - p1->x)};

            float min_ply1 = INFINITY;
            float max_ply1 = -INFINITY;
            for (size_t i = 0; i < a->point_count; i++){
                sp_vec2 sp = a->points[i];
                float d = vec2_dot(sp, axis);
                if(d < min_ply1){
                    min_ply1 = d;
                }
                if(d > max_ply1){
                    max_ply1 = d;
                }
            }
            
            float min_ply2 = INFINITY;
            float max_ply2 = -INFINITY;
            for (size_t i = 0; i < b->point_count; i++){
                sp_vec2 sp = b->points[i];
                float d = vec2_dot(sp, axis);
                if(d < min_ply2){
                    min_ply2 = d;
                }
                if(d > max_ply2){
                    max_ply2 = d;
                }
            }

            if(!(max_ply2 >= min_ply1 && max_ply1 >= min_ply2)){
                a->colliding = 0;
                b->colliding = 0;
                return 0;
            }
            
        } 
    }
    a->colliding = 1;
    b->colliding = 1;
    return 1;
}