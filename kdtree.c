#include "kdtree.h"

#ifdef _MSC_VER
#pragma warning( disable : 4756  )
#endif

int boundingEdgeCompare(const void *a,const void *b) {
  const struct BoundingEdge *aa=(const struct BoundingEdge*)a;
  const struct BoundingEdge *bb=(const struct BoundingEdge*)b;

  if(aa->val==bb->val) {
    return (aa->type < bb->type)?-1:1;
  }

  return (aa->val<bb->val)?-1:1;
}

float calcBoundingBoxSa(const float *start,const float *end) {
  float diag[3];
  diag[0]=end[0]-start[0];
  diag[1]=end[1]-start[1];
  diag[2]=end[2]-start[2];
  return 2.0f*(diag[0]*diag[1]+diag[0]*diag[2]+diag[1]*diag[2]);
}

int calcBoundingBoxMaximumExtent(const float *start,const float *end) {
  float diag[3];

  diag[0]=end[0]-start[0];
  diag[1]=end[1]-start[1];
  diag[2]=end[2]-start[2];

  if (diag[0] > diag[1] && diag[0] > diag[2]) {
    return 0;
  } else if (diag[1] > diag[2]) {
    return 1;
  }

  return 2;
}

bool calcSaCost(int axis,float edget,int nAbove,int nBelow,
                float invTotalSa,const float *minBound,const float *maxBound,
                float emptyBonus,float traversalCost,float isectCost,
                float *cost) {

  float belowSa,aboveSa,pBelow,pAbove,eb,diag[3];
  int otherAxis0,otherAxis1;

  if(!(edget > minBound[axis] && edget < maxBound[axis])) {
    return false;
  }

  diag[0]=maxBound[0]-minBound[0];
  diag[1]=maxBound[1]-minBound[1];
  diag[2]=maxBound[2]-minBound[2];

  otherAxis0=(axis+1)%3;
  otherAxis1=(axis+2)%3;

  belowSa=2.0f*(diag[otherAxis0]*diag[otherAxis1]+
                (edget-minBound[axis])*
                (diag[otherAxis0]+diag[otherAxis1]));
  aboveSa=2.0f*(diag[otherAxis0]*diag[otherAxis1]+
                (maxBound[axis]-edget)*
                (diag[otherAxis0]+diag[otherAxis1]));

  pBelow=belowSa*invTotalSa;
  pAbove=aboveSa*invTotalSa;

  eb=(nAbove==0 || nBelow==0)?emptyBonus:0.0f;
  *cost=traversalCost+isectCost*(1.0f-eb)*
    (pBelow*(float)nBelow+pAbove*(float)nAbove);

  return true;
}

bool findSplitPlane(const struct BoundingBox *bounds,
                    const unsigned int *primInds,int primsNum,
                    const float *minBound,const float *maxBound,
                    float emptyBonus,float traversalCost,float isectCost,
                    int *badRefines,
                    int *axisOut,float *splitOut,
                    unsigned int **leftInds,int *leftIndsNum,
                    unsigned int **rightInds,int *rightIndsNum) {

  int bestAxis,bestOffset,axis,retries,i;
  float bestCost,oldCost,totalSa,invTotalSa;
  struct BoundingEdge *edges[3];

  bestAxis=-1;
  bestOffset=-1;
  bestCost=INFINITY;
  oldCost=isectCost*(float)primsNum;

  totalSa=calcBoundingBoxSa(minBound,maxBound);
  invTotalSa=1.0f/totalSa;
  axis=calcBoundingBoxMaximumExtent(minBound,maxBound);

  edges[0]=0;
  edges[1]=0;
  edges[2]=0;

  for(retries=0;retries<3;retries++) {
    int nBelow,nAbove;

    //
    edges[axis]=(struct BoundingEdge*)
      malloc(sizeof(struct BoundingEdge)*primsNum*2);

    //
    for(i=0;i<primsNum;i++) {
      int v;
      v=primInds[i];

      edges[axis][2*i].type=0;
      edges[axis][2*i].val=bounds[v].start[axis];
      edges[axis][2*i].primInd=v;

      edges[axis][2*i+1].type=1;
      edges[axis][2*i+1].val=bounds[v].end[axis];
      edges[axis][2*i+1].primInd=v;
    }

    qsort(edges[axis],primsNum*2,
          sizeof(struct BoundingEdge),
          boundingEdgeCompare);

    //
    nBelow=0;
    nAbove=primsNum;


    //
    for(i=0;i<2*primsNum;i++) {
      float edget,cost;

      if(edges[axis][i].type==1) {
        nAbove--;
      }

      edget=edges[axis][i].val;

      if(calcSaCost(axis,edget,nAbove,nBelow,
                    invTotalSa,minBound,maxBound,
                    emptyBonus,traversalCost,isectCost,
                    &cost)) {
        // printf("cost %g\n",(double)cost);
        //update best split if lowest score so far
        if(cost < bestCost) {
          bestCost=cost;
          bestAxis=axis;
          bestOffset=i;
        }
      }

      if(edges[axis][i].type==0) {
        nBelow++;
      }
    }

    //
    if(bestAxis == -1 && retries < 2) {
      axis = (axis+1) % 3;
      continue;
    }

    if(bestCost > oldCost) {
      ++(*badRefines);
    }
    // printf("br %i\n",*badRefines);
    if((bestCost>(4.0f*oldCost) && primsNum<16) ||
       bestAxis == -1
       // || (*badRefines) == 3
       ) {
      free(edges[0]);
      free(edges[1]);
      free(edges[2]);
      return false;
    }

    break;
  }

  *axisOut=bestAxis;
  *splitOut=edges[bestAxis][bestOffset].val;

  *leftInds=0;
  *rightInds=0;
  *leftIndsNum=0;
  *rightIndsNum=0;

  for(i=0;i<bestOffset;i++) {
    if(edges[bestAxis][i].type==0) {
      (*leftIndsNum)++;
    }
  }

  for(i=bestOffset+1;i<2*primsNum;i++) {
    if(edges[bestAxis][i].type==1) {
      (*rightIndsNum)++;
    }
  }

  if(*leftIndsNum != 0) {
    *leftInds=(unsigned int*)malloc(sizeof(int)*(*leftIndsNum));
  }

  if(*rightIndsNum != 0) {
    *rightInds=(unsigned int*)malloc(sizeof(int)*(*rightIndsNum));
  }

  int n0,n1;
  n0=0;
  n1=0;

  for(i=0;i<bestOffset;i++) {
    if(edges[bestAxis][i].type==0) {
      (*leftInds)[n0++]=edges[bestAxis][i].primInd;
    }
  }

  for(i=bestOffset+1;i<2*primsNum;i++) {
    if(edges[bestAxis][i].type==1) {
      (*rightInds)[n1++]=edges[bestAxis][i].primInd;
    }
  }

  free(edges[0]);
  free(edges[1]);
  free(edges[2]);
  return true;
}

float Log2(float x) {
  return logf(x) * (1.0f / logf(2.0f));
}

int Floor2Int(float val) {
  return (int)floorf(val);
}

int Round2Int(float val) {
    return Floor2Int(val + 0.5f);
}

int Log2Int(float v) {
  return Floor2Int(Log2(v));
}
int PowInt(int base, int exp) {
  int result = 1;

  while (exp) {
    if (exp & 1) {
      result *= base;
    }

    exp >>= 1;
    base *= base;
  }

  return result;
}

int calcApropiateKdtreeDepth(int primsNum) {
  return Round2Int(8 + 1.3f * Log2Int((float)primsNum));
}

int calcKdtreeSize(int depth) {
  return PowInt(2,depth-1)*2-1;
}

struct KdNode *buildKdTree(const struct BoundingBox *bounds,
                           int primsNum,
                           const float *minBound,
                           const float *maxBound,
                           int maxDepth) {
int curDepth;
  int i,maxPrimsNum;
  float emptyBonus,isectCost,traversalCost;

  int badRefines;
  struct KdBuildStack *buildStack;
  struct KdNode *rootNode;

  curDepth=0;

  // maxDepth=(int)(8.0+1.3*log2((double)primsNum));

  // maxDepth=24;
  // maxDepth+=6;
  // printf("md %i\n",maxDepth);

  isectCost=80.0f;
  traversalCost=1.0f;

  maxPrimsNum=1;
  badRefines=0;

  emptyBonus=0.5f;


  rootNode=(struct KdNode*)malloc(sizeof(struct KdNode));

  buildStack=(struct KdBuildStack*)malloc(sizeof(struct KdBuildStack));
  buildStack->next=0;
  buildStack->primInds=(unsigned int*)malloc(sizeof(int)*primsNum);
  buildStack->primsNum=primsNum;
  buildStack->node=rootNode;
  buildStack->depth=0;//maxDepth;

  for(i=0;i<primsNum;i++) {
     buildStack->primInds[i]=i;
  }

  //
  for(i=0;i<3;i++) {
    buildStack->minBound[i]=minBound[i];
    buildStack->maxBound[i]=maxBound[i];
  }

  //
  while(buildStack) {
    struct KdBuildStack *curBuild;

    //
    curBuild=buildStack;
    buildStack=buildStack->next;

    //

    if(curBuild->depth > curDepth) {
      curDepth=curBuild->depth;
    }
    //

    // printf("start (%g %g %g) (%g %g %g)\n",
    //        curBuild->minBound[0],curBuild->minBound[1],curBuild->minBound[2],
    //        curBuild->maxBound[0],curBuild->maxBound[1],curBuild->maxBound[2]);

    //
    if(curBuild->primsNum <=maxPrimsNum || curBuild->depth==
       maxDepth
       //0
       ) {
      //make leaf
      curBuild->node->type=3;
      curBuild->node->thing.leaf.primsNum=curBuild->primsNum;
      curBuild->node->thing.leaf.primInds=curBuild->primInds;


      // printf("make leaf (%i) @ %i \n",
      //        curBuild->node->thing.leaf.primsNum,
      //        curBuild->depth);
    } else {
      int axis;
      float split;
      unsigned int *leftInds,*rightInds;
      int leftIndsNum,rightIndsNum;
      bool splitResult;
      splitResult=findSplitPlane(bounds,
                                 curBuild->primInds,curBuild->primsNum,
                                 curBuild->minBound,curBuild->maxBound,
                                 emptyBonus,traversalCost,isectCost,
                                 &badRefines,&axis,&split,
                                 &leftInds,&leftIndsNum,
                                 &rightInds,&rightIndsNum);


      if(splitResult) {
        //make branch


        if(rightIndsNum!=0) {
          struct KdBuildStack *rightBuild;
          struct KdNode *rightNode;

          rightBuild=(struct KdBuildStack*)malloc(sizeof(struct KdBuildStack));
          rightNode=(struct KdNode*)malloc(sizeof(struct KdNode));

          rightBuild->node=rightNode;
          rightBuild->primsNum=rightIndsNum;
          rightBuild->primInds=rightInds;
          rightBuild->depth=curBuild->depth +1;//-1;


          memcpy(rightBuild->minBound,curBuild->minBound,sizeof(float)*3);
          memcpy(rightBuild->maxBound,curBuild->maxBound,sizeof(float)*3);
          rightBuild->minBound[axis]=split;

          rightBuild->next=buildStack;
          buildStack=rightBuild;



          curBuild->node->thing.branch.right=rightNode;
        } else {
          curBuild->node->thing.branch.right=0;
        }

        if(leftIndsNum!=0) {
          struct KdBuildStack *leftBuild;
          struct KdNode *leftNode;

          leftBuild=(struct KdBuildStack*)malloc(sizeof(struct KdBuildStack));
          leftNode=(struct KdNode*)malloc(sizeof(struct KdNode));

          leftBuild->node=leftNode;
          leftBuild->primsNum=leftIndsNum;
          leftBuild->primInds=leftInds;
          leftBuild->depth=curBuild->depth +1;//-1;
          memcpy(leftBuild->minBound,curBuild->minBound,sizeof(float)*3);
          memcpy(leftBuild->maxBound,curBuild->maxBound,sizeof(float)*3);
          leftBuild->maxBound[axis]=split;

          leftBuild->next=buildStack;
          buildStack=leftBuild;



          curBuild->node->thing.branch.left=leftNode;
        } else {
          curBuild->node->thing.branch.left=0;
        }

        curBuild->node->type=axis;
        curBuild->node->thing.branch.split=split;

        free(curBuild->primInds);

      } else {
        //make leaf
        curBuild->node->type=3;
        curBuild->node->thing.leaf.primsNum=curBuild->primsNum;
        curBuild->node->thing.leaf.primInds=curBuild->primInds;

      }
    }

    free(curBuild);
  }

  //


  return rootNode;
}
bool searchKdTree(const float *P, const float *V,
                        const float *invV,
                        float ray_tmax,
                        struct KdTraverseNode *stack,
                        int *stackSize,
                        const struct KdNode *rootNode) {


  while((*stackSize)>0) {
    float tmin,tmax;
    const struct KdNode *node;

    //
    node=stack[(*stackSize)-1].node;
    tmin=stack[(*stackSize)-1].tmin;
    tmax=stack[(*stackSize)-1].tmax;
    (*stackSize)--;


    //
    if(ray_tmax < tmin) {
      return false;
    }

    //

    if(node->type==3) {
      return true;
    }

    //branch


    int axis,belowFirst;
    float tplane,split;
    struct KdNode *firstNode,*secondNode;

    axis=node->type;
    split=node->thing.branch.split;

    // Compute parametric distance along ray to split plane
    tplane=(node->thing.branch.split-P[axis])*invV[axis];



    // Get node children pointers for ray
    //node under plane is always stored after parent
    // node in memory aka left node is under
    belowFirst=(P[axis]<split) || (P[axis] == split && V[axis] <= 0);

    //if below: first=left, second=right
    // else first=right,second=left
    if(belowFirst) {
      firstNode=node->thing.branch.left;
      secondNode=node->thing.branch.right;
    } else {
      firstNode=node->thing.branch.right;
      secondNode=node->thing.branch.left;

    }

    // Advance to next child node, possibly enqueue other child
    if(tplane > tmax
       || tplane <= 0.0f
       ) {
      //push first
      if(firstNode) {
        stack[*stackSize].node=firstNode;
        stack[*stackSize].tmin=tmin;
        stack[*stackSize].tmax=tmax;
        (*stackSize)++;
      }


    } else if (tplane < tmin) {
      //push second
      if(secondNode) {
        stack[*stackSize].node=secondNode;
        stack[*stackSize].tmin=tmin;
        stack[*stackSize].tmax=tmax;
        (*stackSize)++;
      }
    } else {
      // Enqueue _secondChild_ in todo list
      //push second and then first

      if(secondNode) {
        stack[*stackSize].node=secondNode;
        stack[*stackSize].tmin=tplane;
        stack[*stackSize].tmax=tmax;
        (*stackSize)++;
      }
      if(firstNode) {
        stack[*stackSize].node=firstNode;
        stack[*stackSize].tmin=tmin;
        stack[*stackSize].tmax=tplane;
        (*stackSize)++;
      }
    }
  }

  return false;
}
