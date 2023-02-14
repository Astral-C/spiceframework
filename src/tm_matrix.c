#include "tm_matrix.h"
#include <string.h>
#include <math.h>

void tm_mat4_identity(tm_mat4 out){
    memset(out, 0, sizeof(tm_mat4));
    tm_mat4_set(out, 0, 0, 1.0f);
    tm_mat4_set(out, 1, 1, 1.0f);
    tm_mat4_set(out, 2, 2, 1.0f);
    tm_mat4_set(out, 3, 3, 1.0f);
}

void tm_ortho(float left, float right, float top, float bottom, float near, float far, tm_mat4 out){
    memset(out, 0, sizeof(tm_mat4));

    tm_mat4_set(out, 0, 0, 0.5f * (right - left));
    tm_mat4_set(out, 1, 1, 0.5f * (top - bottom));
    tm_mat4_set(out, 2, 2, -0.5f * (far - near));
    tm_mat4_set(out, 3, 3, 1.0f);
    tm_mat4_set(out, 0, 3, -(right+left)/(right-left));
    tm_mat4_set(out, 1, 3, -(top+bottom)/(top-bottom));
    tm_mat4_set(out, 2, 3, -(far+near)/(far-near));

}

void tm_perspective(float fov, float aspect, float near, float far, tm_mat4 out){
    float range = far - near;
    float alpha = tanf((fov * (3.14159265358979323846 / 180)) * 0.5f);

    memset(out, 0, sizeof(tm_mat4));

    tm_mat4_set(out, 0, 0, 1.0f / (alpha * aspect));
    tm_mat4_set(out, 1, 1, 1.0f / alpha);
    tm_mat4_set(out, 2, 2, -(far + near) / range);
    tm_mat4_set(out, 2, 3, -1.0);
    tm_mat4_set(out, 3, 2, -(2.0f * far * near) / range);
}

void tm_lookat(tm_vec3 position, tm_vec3 target, tm_vec3 up, tm_mat4 out){
    tm_vec3 z_axis, x_axis, y_axis;

    memset(out, 0, sizeof(tm_mat4));

    vec3_sub(target, position, &z_axis);
    vec3_norm(&z_axis);

    vec3_cross(up, z_axis, &x_axis);
    vec3_norm(&x_axis);

    vec3_cross(z_axis, x_axis, &y_axis);
    vec3_norm(&y_axis);

    tm_mat4_set(out, 0, 0, x_axis.x);
    tm_mat4_set(out, 0, 1, y_axis.x);
    tm_mat4_set(out, 0, 2, -z_axis.x);

    tm_mat4_set(out, 1, 0, x_axis.y);
    tm_mat4_set(out, 1, 1, y_axis.y);
    tm_mat4_set(out, 1, 2, -z_axis.y);

    tm_mat4_set(out, 2, 0, x_axis.z);
    tm_mat4_set(out, 2, 1, y_axis.z);
    tm_mat4_set(out, 2, 2, -z_axis.z);

    tm_mat4_set(out, 0, 3, -position.x);
    tm_mat4_set(out, 1, 3, -position.y);
    tm_mat4_set(out, 2, 3, position.z);
    tm_mat4_set(out, 3, 3, 1.0f);
}

void tm_mat4_mult_vec4(tm_mat4 mat, tm_vec4 vec, tm_vec4* out){
    tm_vec4 row1 = (tm_vec4) {{ mat[0], mat[4], mat[8], mat[12] }};
    tm_vec4 row2 = (tm_vec4) {{ mat[1], mat[5], mat[9], mat[13] }};
    tm_vec4 row3 = (tm_vec4) {{ mat[2], mat[6], mat[10], mat[14] }};
    tm_vec4 row4 = (tm_vec4) {{ mat[3], mat[7], mat[11], mat[15] }};

    out->x = vec4_dot(vec, row1);
    out->y = vec4_dot(vec, row2);
    out->z = vec4_dot(vec, row3);
    out->w = vec4_dot(vec, row4);
}

void tm_mat4_mult(tm_mat4 a, tm_mat4 b, tm_mat4 out){
    memset(out, 0, sizeof(tm_mat4));

    tm_vec4 a_rows[4] = {
        (tm_vec4) {{ a[0], a[4], a[8], a[12] }},
        (tm_vec4) {{ a[1], a[5], a[9], a[13] }},
        (tm_vec4) {{ a[2], a[6], a[10], a[14] }},
        (tm_vec4) {{ a[3], a[7], a[11], a[15] }}
    };

    tm_vec4 b_cols[4] = {
        (tm_vec4) {{ b[0], b[1], b[2], b[3] }},
        (tm_vec4) {{ b[4], b[5], b[6], b[7] }},
        (tm_vec4) {{ b[8], b[9], b[10], b[11] }},
        (tm_vec4) {{ b[12], b[13], b[14], b[15] }}
    };

    for (size_t x = 0; x < 4; x++){
        for (size_t y = 0; y < 4; y++){
            tm_mat4_set(out, x, y, vec4_dot(a_rows[x], b_cols[y]));
        }        
    }
    

}