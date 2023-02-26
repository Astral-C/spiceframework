#ifndef __SPICE_CAMERA_H__
#define __SPICE_CAMERA_H__
#include <tm_vector.h>
#include <tm_matrix.h>

typedef struct {
    tm_vec3 target;   // The point the camera is orbiting around
    float distance;   // The distance between the camera and the target
    float horizontal_angle;   // The horizontal angle between the camera and the target
    float vertical_angle;   // The vertical angle between the camera and the target
} tm_orbit_camera;

void spiceOrbitCamInit(float fov, float w, float h, float near, float far);

void spiceOrbitCamUpdate();

void spiceOrbitCamGetMVP(tm_mat4 model_matrix, tm_mat4 out);

void spiceOrbitCamGetView(tm_mat4 out);

tm_orbit_camera* spiceGetOrbitCam();

#endif