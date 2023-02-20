#include <spice_cam.h>
#include <tm_matrix.h>
#include <math.h>

void tm_orbit_camera_update_view(tm_orbit_camera* camera, tm_mat4 view) {
    tm_vec3 position;
    tm_vec3 target = camera->target;
    float distance = camera->distance;
    float horizontal_angle = camera->horizontal_angle;
    float vertical_angle = camera->vertical_angle;

    // Calculate the position of the camera based on the target, distance, and angles
    position.x = target.x + distance * sin(horizontal_angle) * cos(vertical_angle);
    position.y = target.y + distance * sin(vertical_angle);
    position.z = target.z + distance * cos(horizontal_angle) * cos(vertical_angle);

    // Calculate the up vector based on the vertical angle
    tm_vec3 up = { 0, cos(vertical_angle), 0 };

    // Calculate the view matrix using the lookat function
    tm_lookat(position, target, up, view);
}
