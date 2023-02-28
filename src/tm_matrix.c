#include "tm_matrix.h"
#include <string.h>
#include <math.h>

#define PI 3.14159265358979323846264338327950288420

void tm_mat4_identity(tm_mat4 out){
	memset(out, 0, sizeof(tm_mat4));
	tm_mat4_set(out, 0, 0, 1.0f);
	tm_mat4_set(out, 1, 1, 1.0f);
	tm_mat4_set(out, 2, 2, 1.0f);
	tm_mat4_set(out, 3, 3, 1.0f);
}

void tm_ortho(float left, float right, float bottom, float top, float near, float far, tm_mat4 out) {
    memset(out, 0, sizeof(tm_mat4));

    tm_mat4_set(out, 0, 0, 2.0f / (right - left));
    tm_mat4_set(out, 1, 1, 2.0f / (top - bottom));
    tm_mat4_set(out, 2, 2, -2.0f / (far - near));
    tm_mat4_set(out, 3, 0, -(right + left) / (right - left));
    tm_mat4_set(out, 3, 1, -(top + bottom) / (top - bottom));
    tm_mat4_set(out, 3, 2, -(far + near) / (far - near));
    tm_mat4_set(out, 3, 3, 1.0f);
}


void tm_perspective(float fov, float aspect, float near, float far, tm_mat4 out) {
	float range = far - near;
	float alpha = tan((fov * (PI/180.0f)) * 0.5);

	memset(out, 0, sizeof(tm_mat4));

	tm_mat4_set(out, 0, 0, (1.0f / (alpha * aspect)));
	tm_mat4_set(out, 1, 1, (1.0f / alpha));
	tm_mat4_set(out, 2, 2, -(far+near) / range);
	tm_mat4_set(out, 3, 2, -(2.0f * (far + near)) / range);
	tm_mat4_set(out, 2, 3, -1.0f);
}

void tm_lookat(const tm_vec3 position, const tm_vec3 target, const tm_vec3 up, tm_mat4 out) {
    tm_vec3 z_axis, x_axis, y_axis, t_up;
    memset(out, 0, sizeof(tm_mat4));

    vec3_sub(target, position, &z_axis);
    vec3_norm(&z_axis);

    vec3_cross(up, z_axis, &x_axis);
    vec3_norm(&x_axis);

    vec3_cross(z_axis, x_axis, &y_axis);
    vec3_norm(&y_axis);

    tm_mat4_set(out, 0, 0, x_axis.x);
    tm_mat4_set(out, 0, 1, y_axis.x);
    tm_mat4_set(out, 0, 2, z_axis.x);

    tm_mat4_set(out, 1, 0, x_axis.y);
    tm_mat4_set(out, 1, 1, y_axis.y);
    tm_mat4_set(out, 1, 2, z_axis.y);

    tm_mat4_set(out, 2, 0, x_axis.z);
    tm_mat4_set(out, 2, 1, y_axis.z);
    tm_mat4_set(out, 2, 2, z_axis.z);

    tm_mat4_set(out, 0, 3, -vec3_dot(x_axis, position));
    tm_mat4_set(out, 1, 3, -vec3_dot(y_axis, position));
    tm_mat4_set(out, 2, 3, vec3_dot(z_axis, position));
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
		(tm_vec4) {{ a[0], a[1], a[2], a[3] }},
		(tm_vec4) {{ a[4], a[5], a[6], a[7] }},
		(tm_vec4) {{ a[8], a[9], a[10], a[11] }},
		(tm_vec4) {{ a[12], a[13], a[14], a[15] }}
	};

	tm_vec4 b_cols[4] = {
		(tm_vec4) {{ b[0], b[4], b[8], b[12] }},
		(tm_vec4) {{ b[1], b[5], b[9], b[13] }},
		(tm_vec4) {{ b[2], b[6], b[10], b[14] }},
		(tm_vec4) {{ b[3], b[7], b[11], b[15] }}
	};

	for (size_t x = 0; x < 4; x++){
		for (size_t y = 0; y < 4; y++){
			tm_mat4_set(out, x, y, vec4_dot(a_rows[x], b_cols[y]));
		}        
	}
}


void tm_mat4_invert(const tm_mat4 src, tm_mat4 dst) {
    float tmp[12]; // temporary storage
    float src_0, src_4, src_8, src_12;
    float det; // determinant

    // calculate pairs for first 8 elements (cofactors)
    tmp[0]  = src[10] * src[15];
    tmp[1]  = src[11] * src[14];
    tmp[2]  = src[9] * src[15];
    tmp[3]  = src[11] * src[13];
    tmp[4]  = src[9] * src[14];
    tmp[5]  = src[10] * src[13];
    tmp[6]  = src[8] * src[15];
    tmp[7]  = src[11] * src[12];
    tmp[8]  = src[8] * src[14];
    tmp[9]  = src[10] * src[12];
    tmp[10] = src[8] * src[13];
    tmp[11] = src[9] * src[12];

    // calculate first 8 elements (cofactors)
    dst[0]  = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7] - tmp[1]*src[5] - tmp[2]*src[6] - tmp[5]*src[7];
    dst[1]  = tmp[1]*src[1] + tmp[6]*src[2] + tmp[9]*src[3] - tmp[0]*src[1] - tmp[7]*src[2] - tmp[8]*src[3];
    dst[2]  = tmp[2]*src[9] + tmp[7]*src[10]+ tmp[10]*src[11]- tmp[3]*src[9] - tmp[6]*src[10]- tmp[11]*src[11];
    dst[3]  = tmp[5]*src[13]+ tmp[8]*src[14]+ tmp[11]*src[15]- tmp[4]*src[13]- tmp[9]*src[14]- tmp[10]*src[15];
    dst[4]  = tmp[1]*src[4] + tmp[2]*src[6] + tmp[5]*src[7] - tmp[0]*src[4] - tmp[3]*src[6] - tmp[4]*src[7];
    dst[5]  = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3] - tmp[1]*src[0] - tmp[6]*src[2] - tmp[9]*src[3];
    dst[6]  = tmp[3]*src[8] + tmp[6]*src[10]+ tmp[11]*src[11]- tmp[2]*src[8] - tmp[7]*src[10]- tmp[10]*src[11];
    dst[7]  = tmp[4]*src[12]+ tmp[9]*src[14]+ tmp[10]*src[15]- tmp[5]*src[12]- tmp[8]*src[14]- tmp[11]*src[15];

    // calculate pairs for second 8 elements (cofactors)
	tmp[0] = src[2] * src[7];
	tmp[1] = src[3] * src[6];
	tmp[2] = src[1] * src[7];
	tmp[3] = src[3] * src[5];
	tmp[4] = src[1] * src[6];
	tmp[5] = src[2] * src[5];
	tmp[6] = src[0] * src[7];
	tmp[7] = src[3] * src[4];
	tmp[8] = src[0] * src[6];
	tmp[9] = src[2] * src[4];
	tmp[10] = src[0] * src[5];
	tmp[11] = src[1] * src[4];

	// calculate second 8 elements (cofactors)
	dst[8]  = tmp[0]*src[13]+ tmp[3]*src[14]+ tmp[4]*src[15]- tmp[1]*src[13]- tmp[2]*src[14]- tmp[5]*src[15];
	dst[9]  = tmp[1]*src[12]+ tmp[6]*src[14]+ tmp[9]*src[15]- tmp[0]*src[12]- tmp[7]*src[14]- tmp[8]*src[15];
	dst[10] = tmp[2]*src[12]+ tmp[7]*src[13]+ tmp[10]*src[15]- tmp[3]*src[12]- tmp[6]*src[13]- tmp[11]*src[15];
	dst[11] = tmp[5]*src[12]+ tmp[8]*src[13]+ tmp[11]*src[14]- tmp[4]*src[12]- tmp[9]*src[13]- tmp[10]*src[14];
	dst[12] = tmp[2]*src[10]+ tmp[5]*src[11]+ tmp[1]*src[9] - tmp[4]*src[11]- tmp[0]*src[9] - tmp[3]*src[10];
	dst[13] = tmp[8]*src[11]+ tmp[0]*src[8] + tmp[7]*src[10]- tmp[6]*src[10]- tmp[9]*src[11]- tmp[1]*src[8];
	dst[14] = tmp[6]*src[9] + tmp[11]*src[11]+ tmp[3]*src[8] - tmp[10]*src[11]- tmp[2]*src[8] - tmp[7]*src[9];
	dst[15] = tmp[10]*src[10]+ tmp[4]*src[8] + tmp[9]*src[9] - tmp[8]*src[10]- tmp[11]*src[9] - tmp[5]*src[8];

	// calculate determinant
	det = src[0] * dst[0] + src[1] * dst[4] + src[2] * dst[8] + src[3] * dst[12];

	// calculate matrix inverse
	det = 1 / det;

	for (int i = 0; i < 16; i++) {
		dst[i] *= det;
	}
}