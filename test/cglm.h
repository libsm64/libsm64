/////////////////////////////////////////////////////////////////////
// A few choice functions and types from the cglm library
// https://github.com/recp/cglm
//
#pragma once

#include <math.h>
#include <string.h>

typedef float vec2[2];
typedef float vec3[3];
typedef float vec4[4];
typedef float mat4[4][4];

static void
glm_mat4_zero( mat4 m )
{
    memset( m, 0, sizeof(mat4) );
}

static void
glm_mat4_identity( mat4 m )
{
    glm_mat4_zero( m );
    m[0][0] = 1.0f;
    m[1][1] = 1.0f;
    m[2][2] = 1.0f;
    m[3][3] = 1.0f;
}

static void
glm_vec4_scale(vec4 v, float s, vec4 dest) {
  dest[0] = v[0] * s;
  dest[1] = v[1] * s;
  dest[2] = v[2] * s;
  dest[3] = v[3] * s;
}

static void
glm_vec4_add(vec4 a, vec4 b, vec4 dest) {
  dest[0] = a[0] + b[0];
  dest[1] = a[1] + b[1];
  dest[2] = a[2] + b[2];
  dest[3] = a[3] + b[3];
}

static void
glm_vec3_scale(vec3 v, float s, vec3 dest) {
  dest[0] = v[0] * s;
  dest[1] = v[1] * s;
  dest[2] = v[2] * s;
}

static void
glm_vec3_sub(vec3 a, vec3 b, vec3 dest) {
  dest[0] = a[0] - b[0];
  dest[1] = a[1] - b[1];
  dest[2] = a[2] - b[2];
}

static float
glm_vec3_dot(vec3 a, vec3 b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static float
glm_vec3_norm2(vec3 v) {
  return glm_vec3_dot(v, v);
}

static float
glm_vec3_norm(vec3 v) {
  return sqrtf(glm_vec3_norm2(v));
}

static void
glm_vec3_normalize(vec3 v) {
  float norm;

  norm = glm_vec3_norm(v);

  if (norm == 0.0f) {
    v[0] = v[1] = v[2] = 0.0f;
    return;
  }

  glm_vec3_scale(v, 1.0f / norm, v);
}

static void
glm_vec3_cross(vec3 a, vec3 b, vec3 dest) {
  /* (u2.v3 - u3.v2, u3.v1 - u1.v3, u1.v2 - u2.v1) */
  dest[0] = a[1] * b[2] - a[2] * b[1];
  dest[1] = a[2] * b[0] - a[0] * b[2];
  dest[2] = a[0] * b[1] - a[1] * b[0];
}

static void
glm_vec3_crossn(vec3 a, vec3 b, vec3 dest) {
  glm_vec3_cross(a, b, dest);
  glm_vec3_normalize(dest);
}

static void
glm_perspective(float fovy,
                float aspect,
                float nearVal,
                float farVal,
                mat4  dest) {
  float f, fn;

  glm_mat4_zero(dest);

  f  = 1.0f / tanf(fovy * 0.5f);
  fn = 1.0f / (nearVal - farVal);

  dest[0][0] = f / aspect;
  dest[1][1] = f;
  dest[2][2] = (nearVal + farVal) * fn;
  dest[2][3] =-1.0f;
  dest[3][2] = 2.0f * nearVal * farVal * fn;
}

static void
glm_translate(mat4 m, vec3 v) {
  vec4 v1, v2, v3;

  glm_vec4_scale(m[0], v[0], v1);
  glm_vec4_scale(m[1], v[1], v2);
  glm_vec4_scale(m[2], v[2], v3);

  glm_vec4_add(v1, m[3], m[3]);
  glm_vec4_add(v2, m[3], m[3]);
  glm_vec4_add(v3, m[3], m[3]);
}

static void
glm_lookat(vec3 eye, vec3 center, vec3 up, mat4 dest) {
  vec3 f, u, s;

  glm_vec3_sub(center, eye, f);
  glm_vec3_normalize(f);

  glm_vec3_crossn(f, up, s);
  glm_vec3_cross(s, f, u);

  dest[0][0] = s[0];
  dest[0][1] = u[0];
  dest[0][2] =-f[0];
  dest[1][0] = s[1];
  dest[1][1] = u[1];
  dest[1][2] =-f[1];
  dest[2][0] = s[2];
  dest[2][1] = u[2];
  dest[2][2] =-f[2];
  dest[3][0] =-glm_vec3_dot(s, eye);
  dest[3][1] =-glm_vec3_dot(u, eye);
  dest[3][2] = glm_vec3_dot(f, eye);
  dest[0][3] = dest[1][3] = dest[2][3] = 0.0f;
  dest[3][3] = 1.0f;
}