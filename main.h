

#include "timer.h"
#include "stb_image_write.h"
#include "tinycthread.h"
#include "jo_jpeg.h"
#include "utils.h"
#include "avilib.h"

#ifdef _MSC_VER
#define R_OK    4
#define W_OK    2
#define F_OK    0
#include <io.h>
#define access _access
#endif

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <html5.h>

void em_canvas_paint_init(int w,int h);
void em_canvas_paint(const unsigned char *pixels);
void em_canvas_paint_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);

#define WORKER_THREADS 0
#else
#define WORKER_THREADS 0
#endif

#ifdef RUN_TEST
#define RAY_MAX_SPLITS 0
#define RAY_MAX_BOUNCES 0
#else
#define RAY_MAX_SPLITS 8
#define RAY_MAX_BOUNCES 1
#endif

#define RAY_MAX_DEPTH (1+RAY_MAX_BOUNCES*((RAY_MAX_SPLITS>0)?RAY_MAX_SPLITS-1:0))

#define DEFAULT_DIFFUSE_REFL 0.3

#define FPS 30

#ifndef M_PI_4
#define M_PI_4		0.78539816339744830962
#endif


struct Scene {
  struct Triangle *triangles;
  struct Sphere *spheres;
  struct PointLight *pointLights;
  int trianglesNum;
  int spheresNum;
  int pointLightsNum;

  struct KdNode *rootNode;
  int kdtreeDepth;
  float minBound[3],maxBound[3];
};
