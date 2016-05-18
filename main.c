#include "main.h"

bool create_video(int canvasWidth, int canvasHeight,
                  struct Scene *scene,float aspectRatio,float yFov,
                  struct Ray **rayStacks,
                  struct KdTraverseNode **traverseStacks)  {
  int canvasSize=canvasWidth*canvasHeight;

  float viewRot[9]={1.0f,0.0f,0.0f,
                    0.0f,1.0f,0.0f,
                    0.0f,0.0f, 1.0f};

  sobol_state_t qrnd;
  sobol_init(&qrnd,2);

  float *canvas;
  canvas=(float*)malloc(sizeof(float)*canvasSize*3);

  //

  unsigned char *bitmap;
  bitmap=(unsigned char*)malloc(canvasSize*3);

  avi_t *avifile;
  createDir("output");
  createDir("output/tmp");
  int fileNum=lastFileNum("output")+1;
  //     printf("fn %i\n",fileNum);
  char fn[512];
  snprintf(fn,sizeof(fn),"output/output%04i.avi",fileNum);

  if(access(fn, F_OK )!=-1) {
    fprintf(stderr,"'%s' already exists.\n",fn);

    return false;
  }



  //
  float viewPosKfs[]={
    2.0f,10.0f,35.0f,
    -9.0f,-10.0f,30.0f,
    -11.0f,10.0f,25.0f,

    12.0f,13.0f,20.0f,
    -12.0f,13.0f,40.0f,
    12.0f,-12.0f,35.0f,

    13.0f,13.0f,40.0f,

    0.0f,0.0f,-10.0f,

    };
  float viewPosInds[]={0.0f,5.0f,10.0f,
                       15.0f,20.0f,25.0f,
                       35.0f,45.0f};


    float frame;
    int count=0;
    for(frame=0.0f;frame<=45.0f;frame+=1.0f/(float)FPS) {

      sobol_init(&qrnd,2);
      float pos[3];
      keyframer(frame,viewPosInds,
                viewPosKfs,8, 3,
                0.0f,true, pos);

      renderScene(0,canvasSize,
                  canvasWidth,canvasHeight,
                  aspectRatio,yFov,
                  scene->minBound,scene->maxBound,
                  pos,
                  viewRot,
                  rayStacks[0],
                  traverseStacks[0],
                  scene->rootNode,
                  scene->triangles,scene->trianglesNum,
                  scene->spheres,scene->spheresNum,
                  scene->pointLights, scene->pointLightsNum,

                  &qrnd,

                  canvas,
                  RAY_MAX_BOUNCES,RAY_MAX_SPLITS
                  );

      //
      int i;
      for(i=0;i<canvasSize;i++) {
        bitmap[i*3]=floatToByte(canvas[i*3]);
        bitmap[i*3+1]=floatToByte(canvas[i*3+1]);
        bitmap[i*3+2]=floatToByte(canvas[i*3+2]);
      }



      char fn2[256];
      snprintf(fn2,sizeof(fn2),"output/tmp/tmp%04i_%i.jpg",fileNum,count);
      jo_write_jpg(fn2,bitmap,canvasWidth,canvasHeight,3, 100);

// #ifdef WIN32
//       char fn3[256];
//       snprintf(fn3,sizeof(fn3),"output/tmp/tmp%04i_%i_w32.bmp",fileNum,count);
//       printf("a\n");
//       // jo_write_jpg(fn3,bitmap,CANVAS_WIDTH,CANVAS_HEIGHT,3, 100);
//       SaveBMPToFile(fn3,bitmap,CANVAS_WIDTH,CANVAS_HEIGHT);
// #endif
      printf("frame %g\n",frame);

      count++;
      //



    }
    int i;
    avifile = AVI_open_output_file(fn);


    if(!avifile) {
      printf("aa\n");
      return false;
    }



    AVI_set_video(avifile,canvasWidth,canvasHeight,(double)FPS,"MJPG");


    for(i=0;i<count;i++) {

      char fn2[256];
      snprintf(fn2,sizeof(fn2),"output/tmp/tmp%04i_%i.jpg",fileNum,i);

      FILE *in;
      unsigned char *buffer=0;
      int buffer_len=0;

      in=fopen(fn2,"rb");

      if (in==0) {
        printf("Cannot open test.jpeg %s\n",fn2);
        return false;
      }

      fseek(in, 0, SEEK_END);
      buffer_len=(int)ftell(in);
      fseek(in,0,SEEK_SET);
      buffer=(unsigned char *)malloc(buffer_len);
      fread(buffer,1,buffer_len,in);
      fclose(in);
      // add_avifile_jpeg_frame( avifile,  bitmap, CANVAS_WIDTH,CANVAS_HEIGHT);
      AVI_write_frame(avifile,(char*)buffer,buffer_len,1);
      free(buffer);
    }

    AVI_close(avifile);

    //

    printf("Wrote '%s'.\n",fn);

    return true;
}

bool create_screenshot(int canvasWidth, int canvasHeight,
                       struct Scene *scene,float aspectRatio,float yFov,
                       struct Ray **rayStacks,
                       struct KdTraverseNode **traverseStacks) {
  int canvasSize=canvasWidth*canvasHeight;

  float viewRot[9]={1.0f,0.0f,0.0f,
                    0.0f,1.0f,0.0f,
                    0.0f,0.0f, 1.0f};

  float viewPos[3]={2.0f,0.0f, 35.0f};
  // float viewPos[3]={0.0f,0.0f, 0.0f};

  sobol_state_t qrnd;
  sobol_init(&qrnd,2);

  float *canvas;
  canvas=(float*)malloc(sizeof(float)*canvasSize*3);

  //

  unsigned char *bitmap;
  bitmap=(unsigned char*)malloc(canvasSize*3);


  //
  createDir("output");
  int fileNum=lastFileNum("output")+1;
  printf("fn %i\n",fileNum);
  char fn[512];
  snprintf(fn,sizeof(fn),"output/output%04i.png",fileNum);

  if(access(fn, F_OK )!=-1) {
    fprintf(stderr,"'%s' already exists.\n",fn);
    return false;
  }
  renderScene(0,canvasSize,
              canvasWidth,canvasHeight,
              aspectRatio,yFov,
              scene->minBound,scene->maxBound,
              viewPos,
              viewRot,
              rayStacks[0],
              traverseStacks[0],
              scene->rootNode,
              scene->triangles,scene->trianglesNum,
              scene->spheres,scene->spheresNum,
              scene->pointLights, scene->pointLightsNum,

              &qrnd,

              canvas,
              RAY_MAX_BOUNCES,RAY_MAX_SPLITS);


  //
  int i;

  for(i=0;i<canvasSize;i++) {
    bitmap[i*3]=floatToByte(canvas[i*3]);
    bitmap[i*3+1]=floatToByte(canvas[i*3+1]);
    bitmap[i*3+2]=floatToByte(canvas[i*3+2]);
  }

  stbi_write_png(fn,canvasWidth,canvasHeight,3,bitmap,canvasWidth*3);

  printf("Wrote '%s'.\n",fn);
  return true;
}

bool create_screenshot2(int canvasWidth, int canvasHeight,
                       struct Scene *scene,float aspectRatio,float yFov,
                       struct Ray **rayStacks,
                       struct KdTraverseNode **traverseStacks) {
  //testing bi directional lighting and no reflections

  int canvasSize=canvasWidth*canvasHeight;

  float viewRot[9]={1.0f,0.0f,0.0f,
                    0.0f,1.0f,0.0f,
                    0.0f,0.0f, 1.0f};

  float viewPos[3]={2.0f,0.0f, 35.0f};
  // float viewPos[3]={0.0f,0.0f, 0.0f};

  sobol_state_t qrnd;
  sobol_init(&qrnd,2);

  float *canvas;
  canvas=(float*)malloc(sizeof(float)*canvasSize*3);

  //

  unsigned char *bitmap;
  bitmap=(unsigned char*)malloc(canvasSize*3);


  //
  createDir("output");
  int fileNum=lastFileNum("output")+1;
  printf("fn %i\n",fileNum);
  char fn[512];
  snprintf(fn,sizeof(fn),"output/output%04i.png",fileNum);

  if(access(fn, F_OK )!=-1) {
    fprintf(stderr,"'%s' already exists.\n",fn);
    return false;
  }
  renderScene2(0,canvasSize,
               canvasWidth,canvasHeight,
               aspectRatio,yFov,
               scene->minBound,scene->maxBound,
               viewPos,
               viewRot,
               rayStacks[0],
               traverseStacks[0],
               scene->rootNode,
               scene->triangles,scene->trianglesNum,
               scene->spheres,scene->spheresNum,
               scene->pointLights, scene->pointLightsNum,

               &qrnd,

               canvas,
               RAY_MAX_BOUNCES,RAY_MAX_SPLITS);


  //
  int i;

  for(i=0;i<canvasSize;i++) {
    bitmap[i*3]=floatToByte(canvas[i*3]);
    bitmap[i*3+1]=floatToByte(canvas[i*3+1]);
    bitmap[i*3+2]=floatToByte(canvas[i*3+2]);
  }

  stbi_write_png(fn,canvasWidth,canvasHeight,3,bitmap,canvasWidth*3);

  printf("Wrote '%s'.\n",fn);
  return true;
}

bool create_gbuffer(int canvasWidth, int canvasHeight,
                    struct Scene *scene,float aspectRatio,float yFov,
                    struct Ray **rayStacks,
                    struct KdTraverseNode **traverseStacks) {
  int canvasSize=canvasWidth*canvasHeight;

  float viewRot[9]={1.0f,0.0f,0.0f,
                    0.0f,1.0f,0.0f,
                    0.0f,0.0f, 1.0f};

  float viewPos[3]={0.0f,0.0f, 50.0f};
  // float viewPos[3]={0.0f,0.0f, 0.0f};


  sobol_state_t qrnd;
  sobol_init(&qrnd,2);

  //
  struct GBufferPixel *canvas;
  canvas=(struct GBufferPixel*)malloc(sizeof(struct GBufferPixel)*canvasSize);


  //
  createDir("output");
  int fileNum=lastFileNum("output")+1;
  printf("fn %i\n",fileNum);

  //
  char allFn[512],colFn[512],norFn[512],depthFn[512];
  snprintf(allFn,sizeof(allFn),"output/output%04i_all.jpg",fileNum);

  snprintf(colFn,sizeof(colFn),"output/output%04i_col.jpg",fileNum);
  snprintf(norFn,sizeof(norFn),"output/output%04i_nor.jpg",fileNum);
  snprintf(depthFn,sizeof(depthFn),"output/output%04i_depth.jpg",fileNum);

  // snprintf(colFn,sizeof(colFn),"deferred/out_col.jpg");
  // snprintf(norFn,sizeof(norFn),"deferred/out_nor.jpg");
  // snprintf(depthFn,sizeof(depthFn),"deferred/out_depth.jpg");

  //
  if(access(allFn,F_OK )!=-1) {
    fprintf(stderr,"'%s' already exists.\n",allFn);
    return false;
  }

  // if(access(colFn,F_OK )!=-1) {
  //   fprintf(stderr,"'%s' already exists.\n",colFn);
  //   return false;
  // }

  // if(access(norFn,F_OK )!=-1) {
  //   fprintf(stderr,"'%s' already exists.\n",norFn);
  //   return false;
  // }

  // if(access(depthFn,F_OK )!=-1) {
  //   fprintf(stderr,"'%s' already exists.\n",depthFn);
  //   return false;
  // }

  //
  renderSceneGBuffer(0,canvasSize,
                     canvasWidth,canvasHeight,
                     aspectRatio,yFov,
                     scene->minBound,scene->maxBound,
                     viewPos,
                     viewRot,
                     rayStacks[0],
                     traverseStacks[0],
                     scene->rootNode,
                     scene->triangles,scene->trianglesNum,
                     scene->spheres,scene->spheresNum,
                     &qrnd,
                     canvas);


  //

  unsigned char *allBitmap,*colBitmap,*norBitmap,*depthBitmap;
  allBitmap=(unsigned char*)malloc(canvasSize*4*3);
  colBitmap=allBitmap;
  norBitmap=&colBitmap[canvasSize*4];
  depthBitmap=&norBitmap[canvasSize*4];

  //
  int i;

  for(i=0;i<canvasSize;i++) {



    colBitmap[i*4+0]=floatToByte(canvas[i].col[0]);
    colBitmap[i*4+1]=floatToByte(canvas[i].col[1]);
    colBitmap[i*4+2]=floatToByte(canvas[i].col[2]);
    colBitmap[i*4+3]=floatToByte(1.0f);


    float sphereNor2[2];
    encodeNorSphereMap(canvas[i].nor,sphereNor2);

    norBitmap[i*4+0]=floatToByte(sphereNor2[0]);
    norBitmap[i*4+1]=floatToByte(sphereNor2[1]);
    norBitmap[i*4+2]=floatToByte(0.0f);
    norBitmap[i*4+3]=floatToByte(1.0f);

    // float sphereNor4[4];
    // EncodeFloatRG(sphereNor2[0],sphereNor4);
    // EncodeFloatRG(sphereNor2[1],&sphereNor4[2]);

    // norBitmap[i*4+0]=floatToByte(sphereNor4[0]);
    // norBitmap[i*4+1]=floatToByte(sphereNor4[1]);
    // norBitmap[i*4+2]=floatToByte(sphereNor4[2]);
    // norBitmap[i*4+3]=floatToByte(sphereNor4[3]);

    // // norBitmap[i*4+0]=floatToByte(canvas[i].nor[0]*0.5f+0.5f);
    // // norBitmap[i*4+1]=floatToByte(canvas[i].nor[1]*0.5f+0.5f);
    // // norBitmap[i*4+2]=floatToByte(canvas[i].nor[2]*0.5f+0.5f);
    // // norBitmap[i*4+3]=floatToByte(1.0f);

    float depth4[4];
    EncodeFloatRGBA(canvas[i].depth,depth4);

    depthBitmap[i*4+0]=floatToByte(depth4[0]);
    depthBitmap[i*4+1]=floatToByte(depth4[1]);
    depthBitmap[i*4+2]=floatToByte(depth4[2]);
    depthBitmap[i*4+3]=floatToByte(depth4[3]);

    // float dd= DecodeFloatRGBA(depth4);


    // dd=(2.0f*1.0f)/(100.0f+1.0f-canvas[i].depth*(100.0f-1.0f));
    // depthBitmap[i*4+0]=floatToByte(dd);
    // depthBitmap[i*4+1]=floatToByte(dd);
    // depthBitmap[i*4+2]=floatToByte(dd);
    // depthBitmap[i*4+3]=255;

    // depthBitmap[i*4+0]=floatToByte(canvas[i].depth);
    // depthBitmap[i*4+1]=floatToByte(canvas[i].depth);
    // depthBitmap[i*4+2]=floatToByte(canvas[i].depth);
    // depthBitmap[i*4+3]=255;
  }

  //

  jo_write_jpg(allFn,allBitmap,canvasWidth,canvasHeight*3,4,100);
  printf("Wrote '%s'.\n",colFn);

  // bitmapFlipVertical(colBitmap,4,canvasWidth,canvasHeight);
  jo_write_jpg(colFn,colBitmap,canvasWidth,canvasHeight,4,100);
  printf("Wrote '%s'.\n",colFn);

  // bitmapFlipVertical(norBitmap,4,canvasWidth,canvasHeight);
  jo_write_jpg(norFn,norBitmap,canvasWidth,canvasHeight,4,100);
  printf("Wrote '%s'.\n",norFn);

  // bitmapFlipVertical(depthBitmap,4,canvasWidth,canvasHeight);
  jo_write_jpg(depthFn,depthBitmap,canvasWidth,canvasHeight,4,100);
  printf("Wrote '%s'.\n",depthFn);

  //
  return true;
}

bool initScene(struct Scene *scene) {

  struct KdNode *rootNode;
  float minBound[3],maxBound[3];
  struct Triangle *triangles=0;
  struct Sphere *spheres=0;
  struct PointLight *pointLights=0;
  int trianglesNum=0;
  int spheresNum=0;
  int pointLightsNum=0;

  struct BoundingBox *bounds;
  int boundsNum;

  //
#ifdef RUN_TEST
  if(!loadMesh_old("data/sibenik_test.oldmesh",&triangles,&trianglesNum)) {
    return false;
  }
#else
  if(!loadMesh_old("data/sibenik.oldmesh",&triangles,&trianglesNum)) {
    return false;
  }
#endif

  spheresNum=1;
  spheres=(struct Sphere*)malloc(sizeof(struct Sphere)*spheresNum);

  spheres[0].material.col[0]=0.6f;
  spheres[0].material.col[1]=0.7f;
  spheres[0].material.col[2]=0.9f;
  spheres[0].material.reflectAmount=0.2f;
  spheres[0].material.shininess=0.1f;
  spheres[0].material.intensity=0.9f;
  spheres[0].material.diffuseRefl=0.3f;
  spheres[0].pos[0]=2.0f;
  spheres[0].pos[1]=-8.0f;
  spheres[0].pos[2]=-12.2f;
  spheres[0].radius=2.0f;

  boundsNum=trianglesNum+spheresNum;

  if(boundsNum==0) {

    return false;
  }

  bounds=(struct BoundingBox*)malloc(sizeof(struct BoundingBox)*boundsNum);
  calcBounds(triangles,trianglesNum,spheres,spheresNum,
             minBound,maxBound,bounds);

  //
  int kdtreeMaxDepth;
  kdtreeMaxDepth=calcApropiateKdtreeDepth(boundsNum);

  rootNode=buildKdTree(bounds,boundsNum,
                       minBound,maxBound,
                       kdtreeMaxDepth);


  if(!rootNode) {
    return false;
  }

  //
  pointLightsNum=2;
  pointLights=(struct PointLight*)
    malloc(sizeof(struct PointLight)*pointLightsNum);

  pointLights[0].col[0]=1.0f;
  pointLights[0].col[1]=1.0f;
  pointLights[0].col[2]=1.0f;
  pointLights[0].pos[0]=3.0f;//-22.0;
  pointLights[0].pos[1]=10.0f;//13.0;
  pointLights[0].pos[2]=-10.0f;//25.0;
  pointLights[0].constAtten=0.8f;
  pointLights[0].linAtten=0.1f;
  pointLights[0].quadAtten=0.0001f;


  pointLights[1].col[0]=1.0f;
  pointLights[1].col[1]=1.0f;
  pointLights[1].col[2]=1.0f;
  pointLights[1].pos[0]=3.0f;//-22.0;
  pointLights[1].pos[1]=10.0f;//13.0;
  pointLights[1].pos[2]=24.0f;//25.0;
  pointLights[1].constAtten=0.8f;
  pointLights[1].linAtten=0.1f;
  pointLights[1].quadAtten=0.0001f;


  //
  scene->triangles=triangles;
  scene->spheres=spheres;
  scene->pointLights=pointLights;
  scene->trianglesNum=trianglesNum;
  scene->spheresNum=spheresNum;
  scene->pointLightsNum=pointLightsNum;

  scene->rootNode=rootNode;
  scene->kdtreeDepth=kdtreeMaxDepth;

  memcpy(scene->minBound,minBound,sizeof(minBound));
  memcpy(scene->maxBound,maxBound,sizeof(maxBound));
  //


  //
  return true;
}

#ifdef __EMSCRIPTEN__
struct EmData {
  int canvasWidth,canvasHeight;
  float aspectRatio,yFov;
  struct Ray **rayStacks;
  struct KdTraverseNode **traverseStacks;
  int pixelIndex;

  struct Scene *scene;
};

void em_main_loop(void *data) {
  struct EmData *emData=(struct EmData*)data;

  int canvasSize=emData->canvasWidth*emData->canvasHeight;

  sobol_state_t qrnd;
  sobol_init(&qrnd,2);
  int i;
  for(i=0;i<emData->canvasWidth;i++) {
    if(emData->pixelIndex>=canvasSize) {
      // emscripten_force_exit(0);
      emscripten_cancel_main_loop();
      return;
    }

    float viewRot[9]={1.0f,0.0f,0.0f,
                    0.0f,1.0f,0.0f,
                    0.0f,0.0f, 1.0f};

  float viewPos[3]={2.0f,0.0f, 35.0f};

  float canvas[3];
  renderScene(emData->pixelIndex,emData->pixelIndex+1,
              emData->canvasWidth,emData->canvasHeight,
              emData->aspectRatio,emData->yFov,
              emData->scene->minBound,emData->scene->maxBound,
              viewPos,
              viewRot,
              emData->rayStacks[0],
              emData->traverseStacks[0],
              emData->scene->rootNode,
              emData->scene->triangles,emData->scene->trianglesNum,
              emData->scene->spheres,emData->scene->spheresNum,
              emData->scene->pointLights, emData->scene->pointLightsNum,

              &qrnd,

              canvas,
              RAY_MAX_BOUNCES,RAY_MAX_SPLITS);



  int x,y;


  x=emData->pixelIndex%emData->canvasWidth;
  y=emData->pixelIndex/emData->canvasWidth;
  em_canvas_paint_pixel(x,y,
                        floatToByte(canvas[0]),
                        floatToByte(canvas[1]),
                        floatToByte(canvas[2]));
  // printf("%i :  %i %i\n",emData->pixelIndex,x,y);
  emData->pixelIndex++;
  }
  // printf("Test %i\n",  emData->pixelIndex);
}
#endif

int main(int argc, char *argv[]) {
  double startTime;
  startTime=timer();

  int canvasWidth=1280;
  int canvasHeight=720;

  int i;

#ifdef __EMSCRIPTEN__

  canvasWidth=640;
  canvasHeight=480;
#else

  char mode='p';


  for(i=1;i<argc;i++) {
    if(argv[i][0]=='-' && argv[i][1]=='m' &&
       (argv[i][2]=='v' || argv[i][2]=='p' || argv[i][2]=='g')
       && argv[i][3]==0) {
      mode=argv[i][2];
    } else if(argv[i][0]=='-' && argv[i][1]=='w') {
      canvasWidth=atoi(&argv[i][2]);
    } else if(argv[i][0]=='-' && argv[i][1]=='h') {
      canvasHeight=atoi(&argv[i][2]);
    } else {
      //err
    }
  }
#endif
  int canvasSize=canvasWidth*canvasHeight;




  float aspectRatio=(float)canvasWidth/(float)canvasHeight;
  float yFov=(float)M_PI_4;


  struct Scene scene;

  if(!initScene(&scene)) {
#ifdef _MSC_VER
  system("pause");
#endif

    return 1;
  }


  //

  struct Ray **rayStacks;
  struct KdTraverseNode **traverseStacks;

  rayStacks=(struct Ray**)malloc(sizeof(void*)*(WORKER_THREADS+1));
  traverseStacks=(struct KdTraverseNode**)malloc(sizeof(void*)*(WORKER_THREADS+1));

  for(i=0;i<WORKER_THREADS+1;i++) {
    traverseStacks[i]=(struct KdTraverseNode*)
      malloc(sizeof(struct KdTraverseNode)*scene.kdtreeDepth);
    rayStacks[i]=(struct Ray*)malloc(sizeof(struct Ray)*RAY_MAX_DEPTH);
  }



  //
  printf("Init took %g seconds.\n",timer()-startTime);
  startTime=timer();


#ifdef __EMSCRIPTEN__
  struct EmData emData;
 emData.canvasWidth=canvasWidth;
 emData.canvasHeight=canvasHeight;
 emData.aspectRatio=aspectRatio;
 emData.yFov=yFov;
 emData.traverseStacks=traverseStacks;
 emData.rayStacks=rayStacks;
 emData.pixelIndex=0;
 emData.scene=&scene;
 emscripten_set_canvas_size(canvasWidth,canvasHeight);
 em_canvas_paint_init(canvasWidth,canvasHeight);

 emscripten_set_main_loop_arg(em_main_loop,&emData,0,1);
#else
 //
 if(mode=='v') {
   printf("generating video.\n");
   create_video(canvasWidth,canvasHeight,&scene,aspectRatio,yFov,
                rayStacks,traverseStacks
                );
 } else if(mode=='p') {
   printf("generating picture.\n");
   create_screenshot(canvasWidth,canvasHeight,&scene,aspectRatio,yFov,
                     rayStacks,traverseStacks
                     );
 } else if(mode=='b') {
   printf("generating picture (bi directional lighting test).\n");
   create_screenshot2(canvasWidth,canvasHeight,&scene,aspectRatio,yFov,
                     rayStacks,traverseStacks
                     );
 } else if(mode=='g') {
   printf("generating gbuffer.\n");
   create_gbuffer(canvasWidth,canvasHeight,&scene,aspectRatio,yFov,
                  rayStacks,traverseStacks);
 }

#endif

 printf("Output took %g seconds.\n",timer()-startTime);



#ifdef _MSC_VER
  system("pause");
#endif
  return 0;
}
