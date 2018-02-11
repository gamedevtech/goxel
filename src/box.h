/* Goxel 3D voxels editor
 *
 * copyright (c) 2015 Guillaume Chereau <guillaume@noctua-software.com>
 *
 * Goxel is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.

 * Goxel is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.

 * You should have received a copy of the GNU General Public License along with
 * goxel.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BOX_H
#define BOX_H

#include "vec.h"

// A Box is represented as the 4x4 matrix that transforms the unit cube into
// the box.
typedef union {
    float mat[4][4];
    struct {
        float w[3]; float w_;
        float h[3]; float h_;
        float d[3]; float d_;
        float p[3]; float p_;
    };
    float v[4][4];
} box_t;

static inline bool box_is_bbox(const float b[4][4])
{
    int i, j;
    for (i = 0; i < 3; i++)
    for (j = 0; j < 4; j++) {
        if (mat4_identity[i][j] == 0 && b[i][j] != 0)
            return false;
    }
    return true;
}

static inline box_t bbox_from_extents(const float pos[3],
                                      float hw, float hh, float hd)
{
    box_t ret;
    mat4_set_identity(ret.mat);
    vec3_copy(pos, ret.p);
    ret.w[0] = hw;
    ret.h[1] = hh;
    ret.d[2] = hd;
    return ret;
}

static const box_t box_null = {
    {{0, 0, 0, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}}
};

static inline bool box_is_null(box_t b)
{
    return b.v[3][3] == 0;
}

static inline box_t bbox_from_aabb(const int aabb[2][3])
{
    const float pos[3] = {(aabb[1][0] + aabb[0][0]) / 2.f,
                          (aabb[1][1] + aabb[0][1]) / 2.f,
                          (aabb[1][2] + aabb[0][2]) / 2.f};
    const float size[3] = {(float)(aabb[1][0] - aabb[0][0]),
                           (float)(aabb[1][1] - aabb[0][1]),
                           (float)(aabb[1][2] - aabb[0][2])};
    return bbox_from_extents(pos, size[0] / 2, size[1] / 2, size[2] / 2);
}

static inline void bbox_to_aabb(box_t b, int aabb[2][3])
{
    aabb[0][0] = round(b.p[0] - b.w[0]);
    aabb[0][1] = round(b.p[1] - b.h[1]);
    aabb[0][2] = round(b.p[2] - b.d[2]);
    aabb[1][0] = round(b.p[0] + b.w[0]);
    aabb[1][1] = round(b.p[1] + b.h[1]);
    aabb[1][2] = round(b.p[2] + b.d[2]);
}


// XXX: remove?
static inline box_t bbox_from_points(const float a[3], const float b[3])
{
    float v0[3], v1[3], mid[3];
    v0[0] = min(a[0], b[0]);
    v0[1] = min(a[1], b[1]);
    v0[2] = min(a[2], b[2]);
    v1[0] = max(a[0], b[0]);
    v1[1] = max(a[1], b[1]);
    v1[2] = max(a[2], b[2]);
    vec3_mix(v0, v1, 0.5, mid);
    return bbox_from_extents(mid, (v1[0] - v0[0]) / 2,
                                  (v1[1] - v0[1]) / 2,
                                  (v1[2] - v0[2]) / 2);
}

static inline box_t bbox_from_npoints(int n, const float (*points)[3])
{
    assert(n >= 1);
    int i;
    float v0[3], v1[3], mid[3];
    vec3_copy(points[0], v0);
    vec3_copy(points[0], v1);
    for (i = 1; i < n; i++) {
        v0[0] = min(v0[0], points[i][0]);
        v0[1] = min(v0[1], points[i][1]);
        v0[2] = min(v0[2], points[i][2]);
        v1[0] = max(v1[0], points[i][0]);
        v1[1] = max(v1[1], points[i][1]);
        v1[2] = max(v1[2], points[i][2]);
    }
    vec3_mix(v0, v1, 0.5, mid);
    return bbox_from_extents(mid, (v1[0] - v0[0]) / 2,
                                  (v1[1] - v0[1]) / 2,
                                  (v1[2] - v0[2]) / 2);
}

static inline box_t bbox_intersection(box_t a, box_t b) {
    assert(box_is_bbox(a.mat));
    assert(box_is_bbox(b.mat));
    float a0[3], a1[3], b0[3], b1[3], c0[3], c1[3], mid[3];
    vec3_set(a0, a.p[0] - a.w[0], a.p[1] - a.h[1], a.p[2] - a.d[2]);
    vec3_set(a1, a.p[0] + a.w[0], a.p[1] + a.h[1], a.p[2] + a.d[2]);
    vec3_set(b0, b.p[0] - b.w[0], b.p[1] - b.h[1], b.p[2] - b.d[2]);
    vec3_set(b1, b.p[0] + b.w[0], b.p[1] + b.h[1], b.p[2] + b.d[2]);
    vec3_set(c0, max(a0[0], b0[0]), max(a0[1], b0[1]), max(a0[2], b0[2]));
    vec3_set(c1, min(a1[0], b1[0]), min(a1[1], b1[1]), min(a1[2], b1[2]));
    if (c0[0] >= c1[0] || c0[1] > c1[1] || c0[2] > c1[2])
        return box_null;
    vec3_mix(c0, c1, 0.5, mid);
    return bbox_from_extents(mid, (c1[0] - c0[0]) / 2,
                                  (c1[1] - c0[1]) / 2,
                                  (c1[2] - c0[2]) / 2);
}

static inline bool bbox_intersect(const float a[4][4], const float b[4][4]) {
    assert(box_is_bbox(a));
    assert(box_is_bbox(b));
    float a0[3], a1[3], b0[3], b1[3];
    vec3_set(a0, a[3][0] - a[0][0], a[3][1] - a[1][1], a[3][2] - a[2][2]);
    vec3_set(a1, a[3][0] + a[0][0], a[3][1] + a[1][1], a[3][2] + a[2][2]);
    vec3_set(b0, b[3][0] - b[0][0], b[3][1] - b[1][1], b[3][2] - b[2][2]);
    vec3_set(b1, b[3][0] + b[0][0], b[3][1] + b[1][1], b[3][2] + b[2][2]);
    return a0[0] <= b1[0] && b0[0] <= a1[0] &&
           a0[1] <= b1[1] && b0[1] <= a1[1] &&
           a0[2] <= b1[2] && b0[2] <= a1[2];
}

static inline bool bbox_contains(const float a[4][4], const float b[4][4]) {
    assert(box_is_bbox(a));
    assert(box_is_bbox(b));
    float a0[3], a1[3], b0[3], b1[3];
    vec3_set(a0, a[3][0] - a[0][0], a[3][1] - a[1][1], a[3][2] - a[2][2]);
    vec3_set(a1, a[3][0] + a[0][0], a[3][1] + a[1][1], a[3][2] + a[2][2]);
    vec3_set(b0, b[3][0] - b[0][0], b[3][1] - b[1][1], b[3][2] - b[2][2]);
    vec3_set(b1, b[3][0] + b[0][0], b[3][1] + b[1][1], b[3][2] + b[2][2]);
    return (a0[0] <= b0[0] && a1[0] >= b1[0] &&
            a0[1] <= b0[1] && a1[1] >= b1[1] &&
            a0[2] <= b0[2] && a1[2] >= b1[2]);
}

static inline bool box_contains(const float a[4][4], const float b[4][4])
{
    const float PS[8][3] = {
        {-1, -1, +1},
        {+1, -1, +1},
        {+1, +1, +1},
        {-1, +1, +1},
        {-1, -1, -1},
        {+1, -1, -1},
        {+1, +1, -1},
        {-1, +1, -1},
    };
    float p[3];
    int i;
    float imat[4][4];

    mat4_invert(a, imat);
    for (i = 0; i < 8; i++) {
        mat4_mul_vec3(b, PS[i], p);
        mat4_mul_vec3(imat, p, p);
        if (    p[0] < -1 || p[0] > 1 ||
                p[1] < -1 || p[1] > 1 ||
                p[2] < -1 || p[2] > 1)
            return false;
    }
    return true;
}

static inline box_t bbox_merge(box_t a, box_t b)
{
    assert(box_is_bbox(a.mat));
    assert(box_is_bbox(b.mat));

    float a0[3], a1[3], b0[3], b1[3], r0[3], r1[3], mid[3];
    vec3_set(a0, a.p[0] - a.w[0], a.p[1] - a.h[1], a.p[2] - a.d[2]);
    vec3_set(a1, a.p[0] + a.w[0], a.p[1] + a.h[1], a.p[2] + a.d[2]);
    vec3_set(b0, b.p[0] - b.w[0], b.p[1] - b.h[1], b.p[2] - b.d[2]);
    vec3_set(b1, b.p[0] + b.w[0], b.p[1] + b.h[1], b.p[2] + b.d[2]);

    r0[0] = min(a0[0], b0[0]);
    r0[1] = min(a0[1], b0[1]);
    r0[2] = min(a0[2], b0[2]);
    r1[0] = max(a1[0], b1[0]);
    r1[1] = max(a1[1], b1[1]);
    r1[2] = max(a1[2], b1[2]);

    vec3_mix(r0, r1, 0.5, mid);
    return bbox_from_extents(mid, (r1[0] - r0[0]) / 2,
                                  (r1[1] - r0[1]) / 2,
                                  (r1[2] - r0[2]) / 2);
}

static inline bool bbox_contains_vec(const float b[4][4], const float v[3])
{
    assert(box_is_bbox(b));
    float b0[3], b1[3];
    vec3_set(b0, b[3][0] - b[0][0], b[3][1] - b[1][1], b[3][2] - b[2][2]);
    vec3_set(b1, b[3][0] + b[0][0], b[3][1] + b[1][1], b[3][2] + b[2][2]);

    return (b0[0] <= v[0] && b1[0] > v[0] &&
            b0[1] <= v[1] && b1[1] > v[1] &&
            b0[2] <= v[2] && b1[2] > v[2]);
}

static inline box_t box_get_bbox(const float b[4][4])
{
    float p[8][3] = {
        {-1, -1, +1},
        {+1, -1, +1},
        {+1, +1, +1},
        {-1, +1, +1},
        {-1, -1, -1},
        {+1, -1, -1},
        {+1, +1, -1},
        {-1, +1, -1},
    };
    int i;
    for (i = 0; i < 8; i++) {
        mat4_mul_vec3(b, p[i], p[i]);
    }
    return bbox_from_npoints(8, p);
}

static inline box_t bbox_grow(box_t b, float x, float y, float z)
{
    b.w[0] += x;
    b.h[1] += y;
    b.d[2] += z;
    return b;
}

static inline void box_get_size(box_t b, float out[3])
{
    float v[3][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}};
    int i;
    for (i = 0; i < 3; i++) {
        mat4_mul_vec4(b.mat, v[i], v[i]);
        out[i] = vec3_norm(v[i]);
    }
}

static inline box_t box_swap_axis(box_t b, int x, int y, int z)
{
    float m[4][4];
    assert(x >= 0 && x <= 2);
    assert(y >= 0 && y <= 2);
    assert(z >= 0 && z <= 2);
    mat4_copy(b.mat, m);
    vec4_copy(m[x], b.mat[0]);
    vec4_copy(m[y], b.mat[1]);
    vec4_copy(m[z], b.mat[2]);
    return b;
}

// Create a new box with the 4 points opposit to the face f and the
// new point.
static inline box_t box_move_face(box_t b, int f, const float p[3])
{
    const float PS[8][3] = {
        {-1, -1, -1},
        {+1, -1, -1},
        {+1, -1, +1},
        {-1, -1, +1},
        {-1, +1, -1},
        {+1, +1, -1},
        {+1, +1, +1},
        {-1, +1, +1},
    };
    const int FS[6][4] = {
        {0, 1, 2, 3},
        {5, 4, 7, 6},
        {0, 4, 5, 1},
        {2, 6, 7, 3},
        {1, 5, 6, 2},
        {0, 3, 7, 4}
    };
    const int FO[6] = {1, 0, 3, 2, 5, 4};
    float ps[5][3];
    int i;

    // XXX: for the moment we only support bbox, but we could make the
    // function generic.
    assert(box_is_bbox(b.mat));
    f = FO[f];
    for (i = 0; i < 4; i++)
        mat4_mul_vec3(b.mat, PS[FS[f][i]], ps[i]);
    vec3_copy(p, ps[4]);
    return bbox_from_npoints(5, ps);
}

static inline float box_get_volume(box_t box)
{
    // The volume is the determinant of the 3x3 matrix of the box
    // time 8 (because the unit cube has a volume of 8).
    float *v = &box.mat[0][0];
    float a, b, c, d, e, f, g, h, i;
    a = v[0]; b = v[1]; c = v[2];
    d = v[4]; e = v[5]; f = v[6];
    g = v[8]; h = v[9]; i = v[10];
    return 8 * fabs(a*e*i + b*f*g + c*d*h - c*e*g - b*d*i - a*f*h);
}

static inline void box_get_vertices(box_t box, float vertices[8][3])
{
    int i;
    const float P[8][3] = {
        {-1, -1, +1},
        {+1, -1, +1},
        {+1, +1, +1},
        {-1, +1, +1},
        {-1, -1, -1},
        {+1, -1, -1},
        {+1, +1, -1},
        {-1, +1, -1},
    };
    for (i = 0; i < 8; i++) {
        mat4_mul_vec3(box.mat, P[i], vertices[i]);
    }
}

static inline box_t bbox_from_box(box_t b)
{
    float vertices[8][3];
    box_get_vertices(b, vertices);
    return bbox_from_npoints(8, vertices);
}

#endif // BOX_H
