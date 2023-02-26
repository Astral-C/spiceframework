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
    tm_vec3 position;
    tm_vec3 target = orbit_camera.target;
    float distance = orbit_camera.distance;
    float horizontal_angle = orbit_camera.horizontal_angle;
    float vertical_angle = orbit_camera.vertical_angle;

    // Calculate the position of the camera based on the target, distance, and angles
    position.x = target.x + distance * sin(horizontal_angle) * cos(vertical_angle);
    position.y = target.y + distance * sin(vertical_angle);
    position.z = target.z + distance * cos(horizontal_angle) * cos(vertical_angle);

    // Calculate the up vector based on the vertical angle
    tm_vec3 up = { 0, cos(vertical_angle), 0 };

    // Calculate the view matrix using the lookat function
    tm_lookat(position, target, up, view_matrix);
}

void spiceOrbitCamGetMVP(tm_mat4 model_matrix, tm_mat4 out){
    tm_mat4 model_view;
    tm_mat4_mult(model_matrix, view_matrix, model_view);
    tm_mat4_mult(projection_matrix, model_view, out);
}

void spiceOrbitCamGetView(tm_mat4 out){
    tm_mat4_mult(projection_matrix, view_matrix, out);
}

tm_orbit_camera* spiceGetOrbitCam(){
    return &orbit_camera;
}