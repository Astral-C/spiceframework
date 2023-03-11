#include <stdio.h>
#include <spice_cam.h>
#include <tm_matrix.h>
#include <string.h>
#include <math.h>

static tm_mat4 projection_matrix = {0};
static tm_mat4 view_matrix = {0};

static tm_orbit_camera orbit_camera = {0};

void spiceOrbitCamInit(float fov, float w, float h, float near, float far){
    tm_perspective(fov, w/h, near, far, projection_matrix);
}

void spiceOrbitCamUpdate() {
    tm_vec3 target = orbit_camera.target;
    float distance = orbit_camera.distance;
    float horizontal_angle = orbit_camera.horizontal_angle;
    float vertical_angle = orbit_camera.vertical_angle;

    // Calculate the position of the camera based on the target, distance, and angles
    orbit_camera.position.x = target.x + distance * sin(horizontal_angle) * cos(vertical_angle);
    orbit_camera.position.y = target.y + distance * sin(vertical_angle);
    orbit_camera.position.z = target.z + distance * cos(horizontal_angle) * cos(vertical_angle);

    // Calculate the up vector based on the vertical angle
    tm_vec3 up = { 0, cos(vertical_angle), 0 };

    // Calculate the view matrix using the lookat function
    tm_lookat(orbit_camera.position, target, up, view_matrix);
}

void spiceOrbitCamGetMVP(tm_mat4 model_matrix, tm_mat4 out){
    tm_mat4 model_view;
    tm_mat4_mult(model_matrix, view_matrix, model_view);
    tm_mat4_mult(projection_matrix, model_view, out);
}

void spiceOrbitCamGetView(tm_mat4 out){
    tm_mat4_mult(projection_matrix, view_matrix, out);
}

void spiceOrbitCamRaycast(float mouse_x, float mouse_y, float screen_width, float screen_height, tm_vec3* direction_out){
    tm_mat4 inverse_projection, inverse_view;
    tm_vec4 ray_eye, ray_clip, ray_world;

    tm_mat4_invert(projection_matrix, inverse_projection);
    tm_mat4_invert(view_matrix, inverse_view);

    ray_clip.x = (2.0f * mouse_x) / screen_width - 1.0f;
    ray_clip.y = 1.0f - (2.0f * mouse_y) / screen_height;
    ray_clip.z = 1.0f;
    ray_clip.w = 1.0f;

    tm_mat4_mult_vec4(inverse_projection, ray_clip, &ray_eye);
    ray_eye.z = 1.0f;
    ray_eye.w = 0.0f;
    
    tm_mat4_mult_vec4(inverse_view, ray_eye, &ray_world);

    direction_out->x = ray_world.x;
    direction_out->y = ray_world.y;
    direction_out->z = ray_world.z;

    vec3_norm(direction_out);
}

tm_orbit_camera* spiceGetOrbitCam(){
    return &orbit_camera;
}