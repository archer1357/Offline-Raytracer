#ifndef KDTREE_H
#define KDTREE_H

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifndef _MSC_VER
#include <tgmath.h>
#endif

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

struct BoundingBox {
  float start[3],end[3];
};

struct BoundingEdge {
  int type;
  float val;
  int primInd;
};

struct KdNode {
  int type;
  union {
    struct Branch {
      float split;
      struct KdNode *left,*right;
    } branch;
    struct Leaf {
      unsigned int *primInds;
      int primsNum;
    } leaf;
  } thing;
};
struct KdBuildStack {
  unsigned int *primInds;
  int primsNum;
  struct KdBuildStack *next;
  struct KdNode *node;
  int depth;
  float minBound[3],maxBound[3];

};

struct KdTraverseNode {
  const struct KdNode *node;
  float tmin,tmax;
};

#ifdef __cplusplus
extern "C" {
#endif
  int calcApropiateKdtreeDepth(int primsNum);
  int calcKdtreeSize(int depth);

  struct KdNode *buildKdTree(const struct BoundingBox *bounds,
                             int primsNum,
                             const float *minBound,
                             const float *maxBound,
                             int maxDepth);

  bool searchKdTree(const float *P, const float *V,
                    const float *invV,
                    float ray_tmax,
                    struct KdTraverseNode *stack,
                    int *stackSize,
                    const struct KdNode *rootNode);
#ifdef __cplusplus
}
#endif
#endif
