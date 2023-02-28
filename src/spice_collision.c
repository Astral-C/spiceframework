#include <math.h>
#include <spice_collision.h>
#include <tm_vector.h>

// Allocates a new sp_convex_polygon with point_count points
// Returns pointer to new sp_convex_polygon on success
// Return NULL on fail
sp_convex_polygon* spiceNewPolyConvex(size_t point_count){
    sp_convex_polygon* poly = malloc(sizeof(sp_convex_polygon));
    if(poly == NULL) return NULL;
    poly->points = malloc(sizeof(sp_vec2) * point_count);
    return poly;
}

// Allocates a new sp_convex_polygon with provided points
// Returns pointer to new sp_convex_polygon on success
// Rreturns NULL on fail
sp_convex_polygon* spiceInitPolyConvex(size_t point_count, const sp_vec2* points){
    sp_convex_polygon* poly = malloc(sizeof(sp_convex_polygon));
    if(poly == NULL) return NULL;
    poly->point_count = point_count;
    poly->points = malloc(sizeof(sp_vec2) * point_count);
    poly->colliding = 0;
    memcpy(poly->points, points, sizeof(sp_vec2) * point_count);
    return poly;
}

// Frees provided sp_convex_polygon
void spiceFreePolyConvex(sp_convex_polygon* poly){
    if(poly == NULL) return;
    if(poly->points != NULL){
        free(poly->points);
    }
    free(poly);
}

// Adds a new point to provided sp_convex_polygon
void spicePolyAddPoint(sp_convex_polygon* polygon, sp_vec2 point){
    if(polygon == NULL) return;
    if(polygon->points){
        polygon->points = realloc(polygon->points, sizeof(sp_vec2) * polygon->point_count + 1);
    } else {
        polygon->points = malloc(sizeof(sp_vec2));
    }
    memcpy(&polygon->points[polygon->point_count], &point, sizeof(point));
    polygon->point_count++;
}

// Takes two sp_convex_polygons and checks for collision
// Returns SP_SUCCESS if polygons collide
// Returns SP_FAIL if not
// Returns SP_ERROR if a or b are NULL
sp_status spicePolyCollideSAT(sp_convex_polygon* a, sp_convex_polygon* b){
    if(a == NULL || b == NULL) return SP_ERROR;
    // Based on the function from OLC's video on SAT.
    // if it ain't broke...
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
                return SP_FAIL;
            }
            
        } 
    }
    a->colliding = 1;
    b->colliding = 1;
    return SP_SUCCESS;
}

// Takes an sp_convex_polygon and sp_rect and checks for collision
// Returns SP_SUCCESS if rect and polygon collide
// Returns SP_FAIL if not
// Returns SP_ERROR if a or b are NULL
sp_status spicePolyRectCollide(sp_convex_polygon* a, sp_rect* b){
    if(a == NULL || b == NULL) return SP_ERROR;
    /*
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
                return SP_FAIL;
            }
            
        } 
    }
    a->colliding = 1;
    b->colliding = 1;
    return SP_SUCCESS;
    */
   return SP_FAIL;
}

// Takes two sp_rects and checks for collision
// Returns SP_SUCCESS if rects collide
// Returns SP_FAIL if not
// Returns SP_ERROR if r1 or r2 are NULL
sp_status spiceAABBCollide(sp_rect* r1, sp_rect* r2){
    if(r1 == NULL || r2 == NULL) return SP_ERROR;
    if( r1->position.x > r2->position.x && r1->position.x < r2->position.x + r2->position.w && r1->position.y > r2->position.y && r1->position.h < r2->position.y + r2->position.h ){
        return SP_SUCCESS;
    } else {
        return SP_FAIL;
    }
}

sp_status spiceSphereCollideRay(tm_vec3 ray_origin, tm_vec3 ray_dir, tm_vec3 sphere_center, float sphere_radius){
    tm_vec3 ray_origin_to_sphere_center;

    vec3_sub(ray_origin, sphere_center, &ray_origin_to_sphere_center);

    float a = vec3_dot(ray_dir, ray_dir);
    float b = 2.0f * vec3_dot(ray_dir, ray_origin_to_sphere_center);
    float c = vec3_dot(ray_origin_to_sphere_center, ray_origin_to_sphere_center) - sphere_radius * sphere_radius;

    float discriminant = b * b - 4.0f * a * c;

    if(discriminant < 0.0f){
        return SP_FAIL;
    }

    float t1 = (-b - sqrt(discriminant) / (2.0f * a));
    float t2 = (-b + sqrt(discriminant) / (2.0f * a));

    if(t1 >= 0.0f || t2 >= 0.0f){
        return SP_SUCCESS;
    }

    return SP_FAIL;
}