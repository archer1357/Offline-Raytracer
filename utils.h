#ifndef UTILS_H
#define UTILS_H

#include "kdtree.h"
#include "math.h"
#include "tinydir.h"
#include "sobol.h"

#include <stdio.h>

#ifdef _MSC_VER
#pragma warning(disable : 4756)
#include <direct.h> //mkdir
#define snprintf _snprintf
#endif

#ifndef _MSC_VER
#include <unistd.h>
#endif

struct Material {
  float col[3], reflectAmount, shininess, intensity,diffuseRefl;
};

struct Sphere {
  struct Material material;
  float pos[3],radius;
};

struct Triangle {
  struct Material material;
  float pt0[3],pt1[3],pt2[3];
  float nor0[3],nor1[3],nor2[3];
  float plane[4];
};

struct PointLight {
  float col[3];
  float pos[3];
  float constAtten,linAtten,quadAtten;
};

struct Ray {
  float origin[3],vec[3];
  int bounces;
  float ratio;
};

struct GBufferPixel {
  float col[3],nor[3],depth;
};


#ifdef __cplusplus
extern "C" {
#endif

  bool intersectKdtree(const float *rayOrigin,const float *rayVec,
                       const float *minBound,const float *maxBound,
                       struct KdTraverseNode *stack,
                       const struct KdNode *rootNode,
                       float *t2,float *colPt2,int *primInd,
                       float *bcOut,
                       const struct Triangle *triangles,int trianglesNum,
                       const struct Sphere *spheres,int spheresNum);

  bool intersectKdtreeP(const float *rayOrigin,const float *rayVec,
                        float rayMax,
                        const float *minBound,const float *maxBound,
                        struct KdTraverseNode *stack,
                        const struct KdNode *rootNode,
                        const struct Triangle *triangles,int trianglesNum,
                        const struct Sphere *spheres,int spheresNum);

  bool loadMesh_old(const char *fn,
                struct Triangle **trianglesOut,
                int *trianglesNumOut);

  void calcBounds(struct Triangle *triangles,int trianglesNum,
                  struct Sphere *spheres,int spheresNum,
                  float *minBound,float *maxBound,
                  struct BoundingBox *bounds);

  void calc_pointLight(const struct PointLight *pointLight,
                       const float *lightDir,
                       float lightDist,
                       const float *rayDir,
                       float shininess,float intensity,
                       const float *nor,
                       float *outCol,float *outSpec);

  bool intersectSceneObject(const struct Ray *ray,
                            struct KdTraverseNode *traverseStack,
                            const float *minBound,const float *maxBound,
                            struct KdNode *rootNode,
                            struct Triangle *triangles,int trianglesNum,
                            struct Sphere *spheres,int spheresNum,
                            float *colPtOut, float *norOut,
                            struct Material **materialOut);

  void calcLights(const struct Ray *ray,struct Material *material,
                  const float *colPt,const float *nor,
                  struct KdTraverseNode *traverseStack,
                  const float *minBound,const float *maxBound,
                  struct KdNode *rootNode,
                  struct Triangle *triangles,int trianglesNum,
                  struct Sphere *spheres,int spheresNum,
                  struct PointLight *pointLights,int pointLightsNum,
                  float *colOut,float *specOut);

  void renderScene(int updateFrom,int updateTo,
                   int width, int height,
                   float aspect,float yFov,
                   const float *minBound,const float *maxBound,
                   const float *origin,
                   const float *rotMat,
                   struct Ray *rayStack,
                   struct KdTraverseNode *traverseStack,
                   struct KdNode *rootNode,
                   struct Triangle *triangles,int trianglesNum,
                   struct Sphere *spheres,int spheresNum,
                   struct PointLight *pointLights,int pointLightsNum,
                   sobol_state_t *qrnd,
                   float *canvas,
                   int maxBounces,int maxSplits);

  void renderSceneGBuffer(int updateFrom,int updateTo,
                          int width, int height,
                          float aspect,float yFov,
                          const float *minBound,const float *maxBound,
                          const float *origin,
                          const float *rotMat,
                          struct Ray *rayStack,
                          struct KdTraverseNode *traverseStack,
                          struct KdNode *rootNode,
                          struct Triangle *triangles,int trianglesNum,
                          struct Sphere *spheres,int spheresNum,
                          sobol_state_t *qrnd,
                          struct GBufferPixel *canvas);


  void createDir(const char *n);

  int lastFileNum(const char *folderName);

  unsigned char floatToByte(float f);

  void EncodeFloatRGBA(float v,float *out);
  float DecodeFloatRGBA(const float *rgba);
  void EncodeFloatRG(float v, float *out);
  void encodeNorSphereMap(const float *n,float *out);

  float DecodeFloatRG(const float *rg);
  void decodeNorSphereMap(const float *enc,float *out);

  void bitmapFlipVertical(unsigned char *data,int c, int w, int h);


  void renderScene2(int updateFrom,int updateTo,
                    int width, int height,
                    float aspect,float yFov,
                    const float *minBound,const float *maxBound,
                    const float *origin,
                    const float *rotMat,
                    struct Ray *rayStack,
                    struct KdTraverseNode *traverseStack,
                    struct KdNode *rootNode,
                    struct Triangle *triangles,int trianglesNum,
                    struct Sphere *spheres,int spheresNum,
                    struct PointLight *pointLights,int pointLightsNum,
                    sobol_state_t *qrnd,
                    float *canvas,
                    int maxBounces,int maxSplits);

#ifdef __cplusplus
}
#endif

#endif
