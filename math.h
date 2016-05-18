#ifndef MYMATH_H
#define MYMATH_H

#ifndef __cplusplus
#include <stdbool.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>



#ifdef __cplusplus
extern "C" {
#endif

  void vec3_cross(float *out,const float *a,const float *b);
  float vec3_dot(const float *a,const float *b);
  float vec3_length(const float *v);
  void vec3_normal(float *out,const float *v);
  void vec3_add(float *out,const float *a,const float *b);
  void vec3_sub(float *out,const float *a,const float *b);
  void vec3_mulScalar(float *out,const float *v,const float s);

  void mat4_identity(float *out);
  void mat4_transpose(float *out,float *m);
  void mat4_mul(float *out,const float *a,const float *b);
  void mat4_mulVector(float *out,const float *m,const float *v);
  void mat4_inverse(float *out,const float *m);
  void mat4_normal(float *out,const float *m);

  void mat4_toMat3(float *out,const float *m);

  void mat4_translate(float *out,const float *v, bool mul);
  void mat4_scale(float *out,const float *v,bool mul);
  void mat4_rotateX(float *out,float x, bool mul);
  void mat4_rotateY(float *out,float y,bool mul);
  void mat4_rotateZ(float *out,float z,bool mul);
  void mat4_rotateAxis(float *out,const float *axis,
                       float angle,bool norm,bool mul);


  void mat3_mulVector(float *out,const float *m,const float *v);


  void reflect(float *out,const float *II,const float *N);
  float minOf(float x, float y);
  float maxOf(float x, float y);
  float clamp(float x, float minVal, float maxVal);

  float mix(float x, float y, float a);
  float saturate(float x);
  float smoothStep(float a,float b,float x);
  float fract(float x);
  float lerp(float f0,float f1,float u);

  void keyframer(float frame, const float *positions,
                 const float *values, int valuesNum, int vecLen,
                 float repeatGap,bool smooth, float *out);

  bool intersectRayPlane(const float *P,const float *V,
                         const float *plane,float *t);

  bool intersectRaySphere(const float *P,const float *V,
                          const float *C,float r,
                          float *t);

  bool intersectRayTriangle(const float *P,const float *V,
                            const float *Q0,const float *Q1,
                            const float *Q2,
                            const float *plane,
                            float *t,float *colPt,
                            float *bcOut);

  bool intersectRayBox(const float *P,const float *invV,
                       const float *boxMin,const float *boxMax,
                       float *h0,float *h1);

  void planeFromPointNormal(const float *pt,
                            const float *nor,
                            bool norm,
                            float *plane);

  void planeFromTriangle(const float *pt0,const float *pt1,
                         const float *pt2,bool cw,
                         float *plane);

  void triangleBoundingBox(const float *pt0,
                           const float *pt1,
                           const float *pt2,
                           float *start,float *end);

  void sphereBoundingBox(const float *pos,float radius,
                         float *start,float *end);

  int powi(int base, int exp);


  void mat4_frustum(float *out,
                    float left,float right,
                    float bottom,float top,
                    float zNear,float zFar);
  void mat4_infFrustum(float *out,
                       float left,float right,
                       float bottom,float top,
                       float zNear);
  void mat4_perspective_fovx(float *out,
                             float fovx,float aspect,
                             float znear,float zfar);
  void mat4_perspective_fovy(float *out,
                             float fovy,float aspect,
                             float znear,float zfar);
  void mat4_infPerspective_fovx(float *out,
                                float fovx,float aspect,
                                float znear);
  void mat4_infPerspective_fovy(float *out,
                                float fovy,float aspect,
                                float znear);


  void mat4_ortho(float *out,float left,float right,
                  float bottom,float top,float nearVal,
                  float farVal);


  void mat4_translateXYZ(float *out,float x,float y, float z,bool mul);
  void mat4_scaleXYZ(float *out,float x,float y, float z,bool mul);

#ifdef __cplusplus
}
#endif
#endif
