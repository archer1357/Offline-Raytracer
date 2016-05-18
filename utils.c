#include "utils.h"

unsigned char bilinearTexSampling1(unsigned char *d, int width, int height,
                                   float u, float v) {
  //from jonolick.com
  int ww = width-1, hh = height-1;
  float fu = (u + 1000.0f) * ww; // NOTE: assumes less than 1000 repeats
  float fv = (v + 1000.0f) * hh;
  int u1 = ((int)fu) % ww;
  int v1 = ((int)fv) % hh;
  int u2 = (u1 + 1) % ww;
  int v2 = (v1 + 1) % hh;
  float fracu = fu - floorf( fu );
  float fracv = fv - floorf( fv );
  float w[4] = {(1 - fracu) * (1 - fracv), fracu * (1 - fracv), (1 - fracu) * fracv, fracu *  fracv};
  unsigned char c[4] = { d[v1*width + u1], d[v1*width + u2], d[v2*width + u1], d[v2*width + u2] };
  return (unsigned char)(c[0]*w[0] + c[1]*w[1] + c[2]*w[2] + c[3]*w[3]);
}

bool intersectKdtree(const float *rayOrigin,const float *rayVec,
                     const float *minBound,const float *maxBound,
                     struct KdTraverseNode *stack,
                     const struct KdNode *rootNode,
                     float *t2,float *colPt2,int *primInd,
                     float *bcOut,
                     const struct Triangle *triangles,int trianglesNum,
                     const struct Sphere *spheres,int spheresNum) {
  int stackSize=1;
  float invV[3];
  bool hit=false;

  stack[0].node=rootNode;

  *t2=-1.0f;

  invV[0]=1.0f/rayVec[0];
  invV[1]=1.0f/rayVec[1];
  invV[2]=1.0f/rayVec[2];

  if(!intersectRayBox(rayOrigin,invV,
                     minBound,maxBound,
                     &stack[0].tmin,
                     &stack[0].tmax)) {
    return false;
  }

  //
  bool result;

  while((result=searchKdTree(rayOrigin,rayVec,invV,
                             INFINITY,
                             stack,&stackSize,rootNode))) {

    int i;

    for(i=0;i<stack[stackSize].node->thing.leaf.primsNum;i++) {
      int q;
      q=stack[stackSize].node->thing.leaf.primInds[i];


      float colPt[3];

      float t;//,uu,vv;

      if(q<trianglesNum) {
        if( intersectRayTriangle(rayOrigin,rayVec,
                                 triangles[q].pt0,
                                 triangles[q].pt1,
                                 triangles[q].pt2,
                                 triangles[q].plane,
                                 &t,colPt,bcOut)) {

          // if( t <stack[stackSize].tmin) {
          //   // continue;
          // }
          // if( t >stack[stackSize].tmax) {
          //    // continue;
          // }
          if( (t < (*t2) || !hit)) {

            // vec3_mulScalar(colPt,rayVec,t);
            // vec3_add(colPt,colPt,rayOrigin);

            *t2=t;
            colPt2[0]=colPt[0];
            colPt2[1]=colPt[1];
            colPt2[2]=colPt[2];
            *primInd=q;
            hit=true;
          }
        }
      } else {

        if(intersectRaySphere(rayOrigin,rayVec,
                              spheres[q-trianglesNum].pos,
                              spheres[q-trianglesNum].radius,
                              &t)) {

          if( (t < (*t2) || !hit)) {
            vec3_mulScalar(colPt,rayVec,t);
            vec3_add(colPt,colPt,rayOrigin);

            *t2=t;
            colPt2[0]=colPt[0];
            colPt2[1]=colPt[1];
            colPt2[2]=colPt[2];
            *primInd=q;
            hit=true;
          }
        }
      }


      //insert primitives here
    }

    if((*t2) >stack[stackSize].tmin && (*t2) <stack[stackSize].tmax) {
      return true;
    }
  }


  return hit;
}


bool intersectKdtree_test(const float *rayOrigin,const float *rayVec,
                     const float *minBound,const float *maxBound,
                     struct KdTraverseNode *stack,
                     const struct KdNode *rootNode,
                     float *t2,float *colPt2,int *primInd,
                     float *bcOut,
                     const struct Triangle *triangles,int trianglesNum,
                     const struct Sphere *spheres,int spheresNum) {

  //todo: test all objects
  return intersectKdtree(rayOrigin,rayVec,minBound,maxBound,
                              stack,rootNode,t2,colPt2,primInd,bcOut,
                              triangles,trianglesNum,spheres,spheresNum);
}

bool intersectKdtreeP(const float *rayOrigin,const float *rayVec,
                      float rayMax,
                      const float *minBound,const float *maxBound,
                      struct KdTraverseNode *stack,
                      const struct KdNode *rootNode,
                      const struct Triangle *triangles,int trianglesNum,
                      const struct Sphere *spheres,int spheresNum) {
  int stackSize=1;
  float invV[3];
  float bc[3];

  stack[0].node=rootNode;

  invV[0]=1.0f/rayVec[0];
  invV[1]=1.0f/rayVec[1];
  invV[2]=1.0f/rayVec[2];

  if(intersectRayBox(rayOrigin,invV,
                     minBound,maxBound,
                     &stack[0].tmin,
                     &stack[0].tmax
                     )) {

    bool result;

    while((result=searchKdTree(rayOrigin,rayVec,invV,
                               rayMax,
                               stack,&stackSize,rootNode))) {
      int i;

      for(i=0;i<stack[stackSize].node->thing.leaf.primsNum;i++) {
        int q;
        float colPt[3];
        float t;

        q=stack[stackSize].node->thing.leaf.primInds[i];

        if(q<trianglesNum) {
          if(intersectRayTriangle(rayOrigin,rayVec,
                                  triangles[q].pt0,
                                  triangles[q].pt1,
                                  triangles[q].pt2,
                                  triangles[q].plane,
                                  &t,colPt,bc)) {
            if(t <= rayMax) {
              return true;
            }
          }
        } else {
          if(intersectRaySphere(rayOrigin,rayVec,
                                spheres[q-trianglesNum].pos,
                                spheres[q-trianglesNum].radius,
                                &t)) {
            if(t <= rayMax) {
              return true;
            }
          }
        }

        //insert primitives here
      }
    }
  }

  return false;
}


bool loadMesh_old(const char *fn,
              struct Triangle **trianglesOut,
              int *trianglesNumOut) {
  FILE *file;
  int dataLen;
  char *data;

  file = fopen(fn, "rb");

  if(!file) {
    fprintf(stderr,"loadMesh read file error : %s\n",fn);
    return false;
  }

  fseek(file,0L,SEEK_END);
  dataLen = ftell(file);
  fseek(file,0L,SEEK_SET);
  data=(char*)malloc(dataLen);
  fread(data,1,dataLen,file);
  fclose(file);

  //
  struct Triangle *triangles;
  int trianglesNum,i;
  trianglesNum=dataLen/((9+9+3+2)*4);

  //
  triangles=(struct Triangle*)malloc(sizeof(struct Triangle)*trianglesNum);

  float *data2=(float*)data;

  for(i=0;i<trianglesNum;i++) {
    struct Triangle *triangle=&triangles[i];

    triangle->pt0[0]=*(data2++);
    triangle->pt0[1]=*(data2++);
    triangle->pt0[2]=*(data2++);

    triangle->nor0[0]=*(data2++);
    triangle->nor0[1]=*(data2++);
    triangle->nor0[2]=*(data2++);

    triangle->pt1[0]=*(data2++);
    triangle->pt1[1]=*(data2++);
    triangle->pt1[2]=*(data2++);

    triangle->nor1[0]=*(data2++);
    triangle->nor1[1]=*(data2++);
    triangle->nor1[2]=*(data2++);

    triangle->pt2[0]=*(data2++);
    triangle->pt2[1]=*(data2++);
    triangle->pt2[2]=*(data2++);

    triangle->nor2[0]=*(data2++);
    triangle->nor2[1]=*(data2++);
    triangle->nor2[2]=*(data2++);
    // printf("p %g %g %g, %g %g %g, %g %g %g\n",
    //   triangle->pt0[0],triangle->pt0[1],triangle->pt0[2],
    //   triangle->pt1[0],triangle->pt1[1],triangle->pt1[2],
    //   triangle->pt2[0],triangle->pt2[1],triangle->pt2[2]
    //   );
    // printf("n %g %g %g, %g %g %g, %g %g %g\n",
    //   triangle->nor0[0],triangle->nor0[1],triangle->nor0[2],
    //   triangle->nor1[0],triangle->nor1[1],triangle->nor1[2],
    //   triangle->nor2[0],triangle->nor2[1],triangle->nor2[2]
    //   );

    planeFromTriangle(triangle->pt0,
                      triangle->pt1,
                      triangle->pt2,
                      true,
                      triangle->plane);


    //float E0[3],E1[3],N[3];
    //vec3_sub(E0,triangle->pt1,triangle->pt0);
    // vec3_sub(E1,triangle->pt2,triangle->pt0);
    // vec3_cross(N,E0,E1);
    // // vec3_normal(N,N);

    //
    // planeFromPointNormal(triangle->pt0,N,false,plane);

    //
    triangle->material.col[0]=*(data2++);
    triangle->material.col[1]=*(data2++);
    triangle->material.col[2]=*(data2++);
    triangle->material.reflectAmount=0.5f;

    triangle->material.shininess=*(data2++);
    triangle->material.intensity=*(data2++);


    triangle->material.diffuseRefl=(float)0.07f;

    // triangle->material.shininess=1.0f;
    // triangle->material.intensity=1.0f;
  }

  free(data);

  //
  *trianglesOut=triangles;
  *trianglesNumOut=trianglesNum;

  //
  return true;
}
void calcBounds(struct Triangle *triangles,int trianglesNum,
                struct Sphere *spheres,int spheresNum,
                float *minBound,float *maxBound,
                struct BoundingBox *bounds) {
  int i,j;

  //triangle bounds
  for(i=0;i<trianglesNum;i++) {
    triangleBoundingBox(triangles[i].pt0,
                        triangles[i].pt1,
                        triangles[i].pt2,
                        bounds[i].start,
                        bounds[i].end);
  }

  //sphere bounds
  for(i=0;i<spheresNum;i++) {
    sphereBoundingBox(spheres[i].pos,spheres[i].radius,
                      bounds[trianglesNum+i].start,
                      bounds[trianglesNum+i].end);
  }

  //insert primitives here

  //min,max bounds
  if(trianglesNum+spheresNum > 0) {
    for(i=0;i<3;i++) {
      minBound[i]=bounds[0].start[i];
      maxBound[i]=bounds[0].end[i];

      for(j=1;j<trianglesNum+spheresNum;j++) {
        if(bounds[j].start[i] < minBound[i]) {
          minBound[i]=bounds[j].start[i];
        }

        if(bounds[j].end[i] > maxBound[i]) {
          maxBound[i]=bounds[j].end[i];
        }
      }
    }
  }
}

void calc_pointLight(const struct PointLight *pointLight,
                     const float *toLightDir,
                     float lightDist,
                     const float *rayDir,
                     float shininess,float intensity,
                     const float *nor,
                     float *outCol,float *outSpec) {

  float fromLightDir[3],atten,eyeDir[3],reflectVec[3],NdotL,spec,diffuse;

  //
  vec3_mulScalar(fromLightDir,toLightDir,-1.0f);

  atten = 1.0f/(pointLight->constAtten+
                pointLight->linAtten*lightDist+
                pointLight->quadAtten*lightDist*lightDist);

  vec3_mulScalar(eyeDir,rayDir,-1.0f);

  reflect(reflectVec,fromLightDir,nor);

  NdotL = maxOf(0.0f, vec3_dot(nor, toLightDir));

  spec=0.0f;

  if(NdotL > 0.0f) {
    float EdotR;

    EdotR=maxOf(0.0f, vec3_dot(eyeDir, reflectVec));
    spec+=powf(EdotR,shininess*128.0f) *intensity;
    spec*=atten;
  }

  diffuse=NdotL*atten;

  outCol[0]=pointLight->col[0]*diffuse;
  outCol[1]=pointLight->col[1]*diffuse;
  outCol[2]=pointLight->col[2]*diffuse;

  *outSpec=spec;
}




bool intersectSceneObject(const struct Ray *ray,
                          struct KdTraverseNode *traverseStack,
                          const float *minBound,const float *maxBound,
                          struct KdNode *rootNode,
                          struct Triangle *triangles,int trianglesNum,
                          struct Sphere *spheres,int spheresNum,
                          float *colPtOut, float *norOut,
                          struct Material **materialOut) {

  float colPt[3],nor[3],t;//,t2;
  struct Material *material=0;
  float colPt2[3];
  float bc[3];
  int primInd;

  // t2=-1.0;

#ifdef RUN_TEST
  if(!intersectKdtree_test(ray->origin,ray->vec,minBound,maxBound,
                           traverseStack,rootNode,&t,colPt2,&primInd,bc,
                           triangles,trianglesNum,spheres,spheresNum)) {
    return false;
  }
#else
  if(!intersectKdtree(ray->origin,ray->vec,minBound,maxBound,
    traverseStack,rootNode,&t,colPt2,&primInd,bc,
    triangles,trianglesNum,spheres,spheresNum)) {
  return false;
}
#endif

  // if(!kdtreeblaa(ray->origin,ray->vec,minBound,maxBound,
               // traverseStack,rootNode,&t,colPt2,&primInd,bc,
                 // triangles)) {
    // return false;
  // }

  // if(t < t2 || t2 <0) {
  //   t2=t;

    colPt[0]=colPt2[0];
    colPt[1]=colPt2[1];
    colPt[2]=colPt2[2];

    if(primInd<trianglesNum) {//triangle

      //nor
      nor[0]=triangles[primInd].plane[0];
      nor[1]=triangles[primInd].plane[1];
      nor[2]=triangles[primInd].plane[2];

      // float aa[3],bb[3];
      // vec3_sub(aa,sceneTriangles[q].nor1,sceneTriangles[q].nor0);
      // vec3_sub(bb,sceneTriangles[q].nor2,sceneTriangles[q].nor0);
      // vec3_mulScalar(aa,aa,bc[0]);
      // vec3_mulScalar(bb,bb,bc[1]);
      // vec3_add(nor,sceneTriangles[q].nor0,aa);
      // vec3_add(nor,nor,bb);
      // vec3_normal(nor,nor);

      // nor[0]=sceneTriangles[q].nor0[0];
      // nor[1]=sceneTriangles[q].nor0[1];
      // nor[2]=sceneTriangles[q].nor0[2];
      // printf("%g %g %g\n",nor[0],nor[1],nor[2]);
      //
      // N1 + uu * (N2 - N1) + vv * (N3 - N1);
      // printf("bc %g %g %g\n",bc[0],bc[1],bc[2]);



      ////////////////
      float nn0[3],nn1[3],nn2[3];
      vec3_mulScalar(nn0,triangles[primInd].nor0,bc[2]);
      vec3_mulScalar(nn1,triangles[primInd].nor1,bc[0]);
      vec3_mulScalar(nn2,triangles[primInd].nor2,bc[1]);
      vec3_add(nor,nn0,nn1);
      vec3_add(nor,nor,nn2);
      /////////////



      // vec3_normal(nor,nor);

      // nor[0]=sceneTriangles[q].nor0[0];
      // nor[1]=sceneTriangles[q].nor0[1];
      // nor[2]=sceneTriangles[q].nor0[2];
      //

      //
      material=&triangles[primInd].material;

    } else {//sphere
      struct Sphere *sphere;
      sphere=&spheres[primInd-trianglesNum];

      vec3_sub(nor,colPt,sphere->pos);
      vec3_normal(nor,nor);

      material=&sphere->material;

    }

    //insert primitives here

    if(!material) {
      return false;
    }
  // }

  // if(t2==-1.0) {
  //   return false;
  // }

  colPtOut[0]=colPt[0];
  colPtOut[1]=colPt[1];
  colPtOut[2]=colPt[2];

  norOut[0]=nor[0];
  norOut[1]=nor[1];
  norOut[2]=nor[2];

  *materialOut=material;

  return true;
}

void calcLights(const struct Ray *ray,struct Material *material,
                const float *colPt,const float *nor,
                struct KdTraverseNode *traverseStack,
                const float *minBound,const float *maxBound,
                struct KdNode *rootNode,
                struct Triangle *triangles,int trianglesNum,
                struct Sphere *spheres,int spheresNum,
                struct PointLight *pointLights,int pointLightsNum,
                float *colOut,float *specOut) {
  //
  int q;
  float resultCol[3]={0.0f,0.0f,0.0f};
  float resultSpec[3]={0.0f,0.0f,0.0f};

  //
  for(q=0;q<pointLightsNum;q++) {
    bool blocked=false;
    float col[3],spec;

    //light dir, dist
    float toLightDir[3],lightDist,invLightDist;
    vec3_sub(toLightDir,pointLights[q].pos,colPt);

    lightDist=vec3_length(toLightDir);
    invLightDist=1.0f/lightDist;
    vec3_mulScalar(toLightDir,toLightDir,invLightDist);

    //

#ifndef RUN_TEST
    if(intersectKdtreeP(colPt,toLightDir,lightDist,
    minBound,maxBound,
                        traverseStack,rootNode,
                        triangles,trianglesNum,spheres,spheresNum)) {
      blocked=true;
    }
#endif

    if(blocked) {
      continue;
    }

    calc_pointLight(&pointLights[q],toLightDir,lightDist,
                    ray->vec,
                    material->shininess,material->intensity,
                    nor,
                    col,&spec);

    resultCol[0]+=col[0]+spec;
    resultCol[1]+=col[1]+spec;
    resultCol[2]+=col[2]+spec;
    resultSpec[0]+=pointLights[q].col[0]*spec;
    resultSpec[1]+=pointLights[q].col[1]*spec;
    resultSpec[2]+=pointLights[q].col[2]*spec;
  }

  colOut[0]=resultCol[0];
  colOut[1]=resultCol[1];
  colOut[2]=resultCol[2];
  specOut[0]=resultSpec[0];
  specOut[1]=resultSpec[1];
  specOut[2]=resultSpec[2];

}
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

                 int maxBounces,int maxSplits) {

  int denom=maxBounces*maxSplits;
  float glossyRatio=(denom!=0)?(1.0f/(float)denom)*0.2f:0.0f;


  float d;
  int i;

  //
  d=1.0f/tanf(yFov/2.0f);

  //
  int raycastsNum;


  for(i=updateFrom;i<updateTo;i++) {
    int x,y;
    float u,v,px,py;
    float pixelCol[3]={0.0f,0.0f,0.0f};

    raycastsNum=1;

    rayStack[raycastsNum-1].bounces=0;
    rayStack[raycastsNum-1].origin[0]=origin[0];
    rayStack[raycastsNum-1].origin[1]=origin[1];
    rayStack[raycastsNum-1].origin[2]=origin[2];
    rayStack[raycastsNum-1].ratio=1.0f;

    x=i%width;
    y=i/width;
    u=x/(float)width;
    v=1.0f-y/(float)height; //flip
    px=2.0f*u-1.0f;
    py=2.0f*v-1.0f;

    float bla[3]={px*aspect,py,-d};//rhs


    mat3_mulVector(rayStack[raycastsNum-1].vec,rotMat,bla);
    vec3_normal(rayStack[raycastsNum-1].vec,rayStack[raycastsNum-1].vec);

    while(raycastsNum > 0) {
      float curCol[3]={0.0f,0.0f,0.0f};
      float nor[3],colPt[3];
      float illumCol[3]={0.0f,0.0f,0.0f};
      float specCol[3]={0.0f,0.0f,0.0f};
      struct Material *material;

      //
      raycastsNum--;
      struct Ray ray=rayStack[raycastsNum];

      if(!intersectSceneObject(&ray,traverseStack,minBound,maxBound,rootNode,
                               triangles,trianglesNum,spheres,spheresNum,
                               colPt,nor,&material)) {
        // pixelCol[0]=1.0f;
        break;
      }

      // pixelCol[0]=material->col[0];
      // pixelCol[1]=material->col[1];
      // pixelCol[2]=material->col[2];
      ////////////////
      calcLights(&ray,material,colPt,nor,
                 traverseStack,minBound,maxBound,rootNode,
                 triangles,trianglesNum,spheres,spheresNum,
                 pointLights,pointLightsNum,illumCol,specCol);

      //
      curCol[0]=(material->col[0]*illumCol[0]+specCol[0]);
      curCol[1]=material->col[1]*illumCol[1]+specCol[1];
      curCol[2]=material->col[2]*illumCol[2]+specCol[2];

      curCol[0]*=ray.ratio;
      curCol[1]*=ray.ratio;
      curCol[2]*=ray.ratio;

      pixelCol[0]+=curCol[0];
      pixelCol[1]+=curCol[1];
      pixelCol[2]+=curCol[2];

      ///////////////
      // pixelCol[0]+=material->col[0];
      // pixelCol[1]+=material->col[1];
      // pixelCol[2]+=material->col[2];


      // pixelCol[0]=(1.0f+nor[0])/2.0f;
      // pixelCol[1]=(1.0f+nor[1])/2.0f;
      // pixelCol[2]=(1.0f+nor[2])/2.0f;

      // pixelCol[0]+=illumCol[0]*0.1f+specCol[0];
      // pixelCol[1]+=illumCol[1]*0.1f+specCol[1];
      // pixelCol[2]+=illumCol[2]*0.1f+specCol[2];

      //pixelCol[0]+=illumCol[0];
      //pixelCol[1]+=illumCol[1];
      //pixelCol[2]+=illumCol[2];




      if(maxSplits !=0 && ray.bounces<maxBounces) {
        float reflectVec[3];
        reflect(reflectVec,ray.vec,nor);
        vec3_normal(reflectVec,reflectVec);

        if(1==maxSplits) {
          rayStack[raycastsNum].origin[0]=colPt[0];
          rayStack[raycastsNum].origin[1]=colPt[1];
          rayStack[raycastsNum].origin[2]=colPt[2];
          //(float)powi(RAY_MAX_SPLITS,RAY_MAX_BOUNCES);

          rayStack[raycastsNum].ratio=ray.ratio*0.15f;
          rayStack[raycastsNum].bounces=ray.bounces+1;

          rayStack[raycastsNum].vec[0]=reflectVec[0];
          rayStack[raycastsNum].vec[1]=reflectVec[1];
          rayStack[raycastsNum].vec[2]=reflectVec[2];
          raycastsNum++;
        } else if(maxSplits > 1) {
          int j;
          float RN1[3],RN2[3],rotMat[9];

          RN1[0]=reflectVec[2];
          RN1[1]=reflectVec[1];
          RN1[2]=-reflectVec[0];
          vec3_cross(RN2,reflectVec,RN1);
          rotMat[0]=RN1[0];
          rotMat[3]=RN1[1];
          rotMat[6]=RN1[2];

          rotMat[1]=RN2[0];
          rotMat[4]=RN2[1];
          rotMat[7]=RN2[2];

          rotMat[2]=reflectVec[0];
          rotMat[5]=reflectVec[1];
          rotMat[8]=reflectVec[2];
          // int x,y;

          // for(x=-3;x<=3;x++) {
          //   for(y=-3;y<=3;y++) {
          //     if(x==0 && y==0) {
          //       //continue;
          //     }
          //     float v[3];
          //     v[0]=(float)x;
          //     v[1]=(float)y;
          //     v[2]=6.0f;

          //     mat3_mulVector(v,rotMat,v);
          //     vec3_normal(v,v);


          //     rayStack[raycastsNum].origin[0]=colPt[0];
          //     rayStack[raycastsNum].origin[1]=colPt[1];
          //     rayStack[raycastsNum].origin[2]=colPt[2];
          //     rayStack[raycastsNum].ratio=ray.ratio*0.01f;
          //     rayStack[raycastsNum].bounces=ray.bounces+1;

          //     rayStack[raycastsNum].vec[0]=v[0];
          //     rayStack[raycastsNum].vec[1]=v[1];
          //     rayStack[raycastsNum].vec[2]=v[2];

          //     raycastsNum++;
          //   }
          // }
          // reflectVec[0]=nor[0];
          // reflectVec[1]=nor[1];
          // reflectVec[2]=nor[2];
          for(j=0;j<maxSplits;j++) {
            float drefl=material->diffuseRefl;
            float xoffs, yoffs;

            do {
              // xoffs =((float)(rand()%1000))/1000.0f * drefl;
              // yoffs = ((float)(rand()%1000))/1000.0f * drefl;


              double v[2];
              sobol_get(qrnd,2,v);


              xoffs=(float)v[0]* drefl;
              yoffs=(float)v[1]* drefl;

            } while((xoffs*xoffs+yoffs*yoffs)>(drefl*drefl));
            float R[3],Ra[3],Rb[3];
            vec3_mulScalar(Ra,RN1,xoffs);
            vec3_mulScalar(Rb,RN2,yoffs*drefl);
            vec3_add(R,reflectVec,Ra);
            vec3_add(R,R,Rb);
            vec3_normal(R,R);



            rayStack[raycastsNum].origin[0]=colPt[0];
            rayStack[raycastsNum].origin[1]=colPt[1];
            rayStack[raycastsNum].origin[2]=colPt[2];
            //(float)powi(RAY_MAX_SPLITS,RAY_MAX_BOUNCES);

            rayStack[raycastsNum].ratio=ray.ratio*glossyRatio;
            rayStack[raycastsNum].bounces=ray.bounces+1;

            rayStack[raycastsNum].vec[0]=R[0];
            rayStack[raycastsNum].vec[1]=R[1];
            rayStack[raycastsNum].vec[2]=R[2];
            raycastsNum++;

          }

          // for(j=0;j<raycastMaxGlossySplits;j++) {
          //   // float vec[3];
          //   // vec3_sub(vec,colPt,raycast.origin);
          //   // vec3_normal(vec,vec);

          //   raycastStack[raycastsNum].origin[0]=colPt[0];
          //   raycastStack[raycastsNum].origin[1]=colPt[1];
          //   raycastStack[raycastsNum].origin[2]=colPt[2];
          //   raycastStack[raycastsNum].ratio=raycast.ratio*0.1f;
          //   raycastStack[raycastsNum].steps=raycast.steps-1;

          //   raycastStack[raycastsNum].vec[0]=reflectVec[0];
          //   raycastStack[raycastsNum].vec[1]=reflectVec[1];
          //   raycastStack[raycastsNum].vec[2]=reflectVec[2];
          //   // reflectf(raycastStack[raycastsNum].vec,vec,nor);

          //   raycastsNum++;
          // }
        }
      }



    }

    //
    canvas[(i-updateFrom)*3]=pixelCol[0];
    canvas[(i-updateFrom)*3+1]=pixelCol[1];
    canvas[(i-updateFrom)*3+2]=pixelCol[2];
  }
}

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
                        struct GBufferPixel *canvas) {

  // float mat[16]={viewRot[0],viewRot[1],viewRot[2],viewPos[0],
  //                    viewRot[3],viewRot[4],viewRot[5],viewPos[1],
  //                    viewRot[6],viewRot[7],viewRot[8],viewPos[2],
  //                    0.0f,      0.0f,      0.0f,      1.0f};

  float viewMat[16]={rotMat[0],rotMat[3],rotMat[6],-origin[0],
                     rotMat[1],rotMat[4],rotMat[7],-origin[1],
                     rotMat[2],rotMat[5],rotMat[8],-origin[2],
                     0.0f,      0.0f,      0.0f,       1.0f};

  float viewRotMat[9]={rotMat[0],rotMat[3],rotMat[6],
                       rotMat[1],rotMat[4],rotMat[7],
                       rotMat[2],rotMat[5],rotMat[8]};
  float znear=20.0f;
  float zfar=100.0f;

  float d;
  int i;

  //
  d=1.0f/tanf(yFov/2.0f);

  //
  int raycastsNum;


  for(i=updateFrom;i<updateTo;i++) {

    int x,y;
    float u,v,px,py;


    raycastsNum=1;

    rayStack[raycastsNum-1].bounces=0;
    rayStack[raycastsNum-1].origin[0]=origin[0];
    rayStack[raycastsNum-1].origin[1]=origin[1];
    rayStack[raycastsNum-1].origin[2]=origin[2];
    rayStack[raycastsNum-1].ratio=1.0f;

    x=i%width;
    y=i/width;
    u=x/(float)width;
    v=1.0f-y/(float)height; //flip
    px=2.0f*u-1.0f;
    py=2.0f*v-1.0f;

    float bla[3]={px*aspect,py,-d};//rhs


    mat3_mulVector(rayStack[raycastsNum-1].vec,rotMat,bla);
    vec3_normal(rayStack[raycastsNum-1].vec,rayStack[raycastsNum-1].vec);

    //



    //
    while(raycastsNum > 0) {
    float nor[3],colPt[3];


    struct Material *material;

    //
    raycastsNum--;
    struct Ray ray=rayStack[raycastsNum];

    if(!intersectSceneObject(&ray,traverseStack,minBound,maxBound,rootNode,
                             triangles,trianglesNum,spheres,spheresNum,
                             colPt,nor,&material)) {
      canvas[i].col[0]=0.0f;
      canvas[i].col[1]=0.0f;
      canvas[i].col[2]=0.0f;
      canvas[i].nor[0]=0.5f;
      canvas[i].nor[1]=0.5f;
      canvas[i].nor[2]=0.5f;
      canvas[i].depth=1.0f;
      break;
    }



    //
     canvas[i].col[0]=material->col[0];
     canvas[i].col[1]=material->col[1];
     canvas[i].col[2]=material->col[2];


     mat3_mulVector(canvas[i].nor,viewRotMat,nor);


     float viewPos[4];
     float pos[4]={colPt[0],colPt[1],colPt[2],1.0f};
     mat4_mulVector(viewPos,viewMat,pos);



     float p10=(zfar+znear)/(znear-zfar);
     float p11=2.0f*zfar*znear/(znear-zfar);

     // float clipZ=p10*viewPos[2]+p11;

     float mvZ=viewPos[2];
     float clipZ=(mvZ*(zfar+znear)+(2.0f*zfar*znear))/(znear-zfar);
     float clipW=-mvZ;
     float ndcZ = clipZ/clipW;

     // if(ndcZ > 1.0f || ndcZ < -1.0f) {
     //   printf("depth %g\n",ndcZ);
     // }

     // float depth=ndcZ*0.5f+0.5f;
     float nearRange=0.0f;
     float farRange=1.0f;
     float depth = (((farRange-nearRange) * ndcZ) + nearRange + farRange) / 2.0f;
     if(depth>1.0f || depth<0.0f) {
       depth=1.0f;

     }
     // if(depth > 1.0f || depth < 0.0f) {
     //   printf("depth %g\n",depth);
     // }
     canvas[i].depth=depth;

     // float linearDepth=(2.0f*znear)/(zfar+znear-depth*(zfar-znear));
     // canvas[i].depth=linearDepth;
     // if(depth<0.98f) {
     //   printf("less\n");
     // }
    }
  }
}

void createDir(const char *n) {
#if defined(_WIN32)
  _mkdir(n);
#elif defined(__linux__)
  mkdir(n, 777);
#endif
}

int lastFileNum(const char *folderName) {
  int num=-1;
  tinydir_dir dir;

  if(tinydir_open(&dir, folderName) != -1) {
    while(dir.has_next) {
      tinydir_file file;
      tinydir_readfile(&dir,&file);
      int fnLen=strlen(file.name);

      char numStr[256];
      numStr[0]='\0';
      memset(numStr,0,256);
      if(!file.is_dir) {
        int i,j=0;

        for(i=0;i<fnLen;i++) {

          // if(file.name[i]=='0' && j==0) {
          //   j++;
          // } else
          if(file.name[i]>='0' && file.name[i]<='9') {
            numStr[j++]=file.name[i];

          } else if(j>0) {
            break;
          }
        }

        if(numStr[0]!='\0') {
          // printf("%s\n",numStr);
          int num2=atoi(numStr);

          if(num2 > num) {
            num=num2;
          }
        }
      }

      tinydir_next(&dir);
    }

    tinydir_close(&dir);
  }

  return num;
}

unsigned char floatToByte(float f) {
  return (unsigned char)(minOf(1.0f,f)*255.0f);
}


float DecodeFloatRGBA(const float *rgba) {
  //dot(rgba, vec4(1.0,1.0/255.0,1.0/65025.0,1.0/16581375.0));
  float x=rgba[0];
  float y=rgba[1]/255.0f;
  float z=rgba[2]/65025.0f;
  float w=rgba[3]/16581375.0f;
  return x+y+z+w;
}
void EncodeFloatRGBA(float v,float *out) {
//from aras-p.info/blog/2009/07/30/encoding-floats-to-rgba-the-final/
  // 255^0 = 1
  // 255^1 = 255
  // 255^2 = 65025
  // 255^3 = 16581375

  float enc[4];

  //vec4 enc = vec4(1.0, 255.0, 65025.0, 16581375.0) * v;
  enc[0]=v;
  enc[1]=v*255.0f;
  enc[2]=v*65025.0f;
  enc[3]=v*16581375.0f;

  //enc-=floor(enc); //enc = frac(enc);
  enc[0]-=floor(enc[0]);
  enc[1]-=floor(enc[1]);
  enc[2]-=floor(enc[2]);
  enc[3]-=floor(enc[3]);

  //enc -= enc.yzww * vec4(1.0/255.0,1.0/255.0,1.0/255.0,0.0);
  out[0]=enc[0]-enc[1]*(1.0f/255.0f);
  out[1]=enc[1]-enc[2]*(1.0f/255.0f);
  out[2]=enc[2]-enc[3]*(1.0f/255.0f);
  out[3]=enc[3];
}

void EncodeFloatRG(float v, float *out) {
  // vec2 kEncodeMul = vec2( 1.0, 255.0);
  float kEncodeMul[2];
  kEncodeMul[0]=1.0f;
  kEncodeMul[1]=255.0f;

  //
  float kEncodeBit = 0.00392157f;

  //
  float enc[2];

  // enc = (kEncodeMul * v);
  enc[0]=kEncodeMul[0]*v;
  enc[1]=kEncodeMul[1]*v;

  //enc = fract( enc );
  enc[0]-=floor(enc[0]);
  enc[1]-=floor(enc[1]);


  // enc.x  -= (enc.y  * kEncodeBit);
  enc[0]-=enc[1]*kEncodeBit;

  out[0]=enc[0];
  out[1]=enc[1];
}

void encodeNorSphereMap(const float *n,float *out) {
  //from aras-p.info/texts/CompactNormalStorage.html
  float f = sqrt(8.0f*n[2]+8.0f);
  out[0]=n[0]/ f + 0.5f;
  out[1]=n[1]/ f + 0.5f;
}

float DecodeFloatRG(const float *enc) {
  float kDecodeDot[2] = { 1.0f, 0.00392157f};
  return enc[0]*kDecodeDot[0]+enc[1]*kDecodeDot[1];

}

void decodeNorSphereMap(const float *enc,float *out) {
  float fenc[2];
  fenc[0]=enc[0]*4.0f-2.0f;
  fenc[1]=enc[1]*4.0f-2.0f;

  float f = fenc[0]*fenc[0]+fenc[1]+fenc[1];
  float g = sqrtf(1.0f-f/4.0f);


  out[0]=fenc[0]*g;
  out[1]=fenc[1]*g;
  out[2]=1.0f-f/2.0f;

}


void bitmapFlipVertical(unsigned char *data,int c, int w, int h) {
  unsigned char *tmp;
  tmp=(unsigned char*)malloc(w*c);
  int i;

  for(i=0;i<h/2;i++) {
    int x=i*c*w;
    int y=(h-1-i)*c*w;
    memcpy(tmp,&data[x],c*w);
    memcpy(&data[x],&data[y],c*w);
    memcpy(&data[y],tmp,c*w);
  }

  free(tmp);
}

bool stringFromFile(const char *fn, char **str) {
  FILE *file;
  unsigned int dataSize;

  //open file
  file = fopen(fn, "rb");

  if(!file) {
    return false;
  }

  //get data size
  fseek(file,0L,SEEK_END);
  dataSize = ftell(file);
  fseek(file,0L,SEEK_SET);

  //
  (*str)=(char*)malloc(dataSize+1);

  //read data
  (*str)[dataSize]=0;//is not already null terminated?
  fread((*str),1,dataSize,file);

  //close file
  fclose(file);
  return true;
}



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

                 int maxBounces,int maxSplits) {

  int denom=maxBounces*maxSplits;
  float glossyRatio=(denom!=0)?(1.0f/(float)denom)*0.2f:0.0f;


  float d;
  int i;

  //
  d=1.0f/tanf(yFov/2.0f);

  //
  int raycastsNum;


  for(i=updateFrom;i<updateTo;i++) {
    int x,y;
    float u,v,px,py;
    float pixelCol[3]={0.0f,0.0f,0.0f};

    raycastsNum=1;

    rayStack[raycastsNum-1].bounces=0;
    rayStack[raycastsNum-1].origin[0]=origin[0];
    rayStack[raycastsNum-1].origin[1]=origin[1];
    rayStack[raycastsNum-1].origin[2]=origin[2];
    rayStack[raycastsNum-1].ratio=1.0f;

    x=i%width;
    y=i/width;
    u=x/(float)width;
    v=1.0f-y/(float)height; //flip
    px=2.0f*u-1.0f;
    py=2.0f*v-1.0f;

    float bla[3]={px*aspect,py,-d};//rhs


    mat3_mulVector(rayStack[raycastsNum-1].vec,rotMat,bla);
    vec3_normal(rayStack[raycastsNum-1].vec,rayStack[raycastsNum-1].vec);

    while(raycastsNum > 0) {
      float curCol[3]={0.0f,0.0f,0.0f};
      float nor[3],colPt[3];
      float illumCol[3]={0.0f,0.0f,0.0f};
      float specCol[3]={0.0f,0.0f,0.0f};
      struct Material *material;

      //
      raycastsNum--;
      struct Ray ray=rayStack[raycastsNum];

      if(!intersectSceneObject(&ray,traverseStack,minBound,maxBound,rootNode,
                               triangles,trianglesNum,spheres,spheresNum,
                               colPt,nor,&material)) {

        break;
      }

      calcLights(&ray,material,colPt,nor,
                 traverseStack,minBound,maxBound,rootNode,
                 triangles,trianglesNum,spheres,spheresNum,
                 pointLights,pointLightsNum,illumCol,specCol);

      //
      curCol[0]=(material->col[0]*illumCol[0]+specCol[0]);
      curCol[1]=material->col[1]*illumCol[1]+specCol[1];
      curCol[2]=material->col[2]*illumCol[2]+specCol[2];

      curCol[0]*=ray.ratio;
      curCol[1]*=ray.ratio;
      curCol[2]*=ray.ratio;

      pixelCol[0]+=curCol[0];
      pixelCol[1]+=curCol[1];
      pixelCol[2]+=curCol[2];




      ///here



    }

    //
    canvas[(i-updateFrom)*3]=pixelCol[0];
    canvas[(i-updateFrom)*3+1]=pixelCol[1];
    canvas[(i-updateFrom)*3+2]=pixelCol[2];
  }
}

/*
#ifdef SPOT
uniform float u_spotCosCutoff;
uniform float u_spotExponent;
#endif

#if defined SPOT || defined DIRECTIONAL
uniform vec4 u_lightDir;
#endif

#ifdef SPOT
  vec3 spotDir=u_lightDir.xyz;
  float spotCos = dot(lightDir, -spotDir);

  if(spotCos < u_spotCosCutoff) {
    atten = 0.0;
  } else {
    atten *= pow(spotCos, u_spotExponent);
  }
#endif

#ifdef DIRECTIONAL
  vec3 lightDir=normalize(-u_lightDir.xyz);
#endif
*/
