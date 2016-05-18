#include "math.h"
#include <string.h>

#ifdef _MSC_VER
#pragma warning(disable : 4756)
#endif

float InvSqrt(float x){
    float xhalf = 0.5f*x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i>>1);
    x = *(float*)&i;
    x = x*(1.5f - xhalf*x*x);
    return x;
}

//===vec3

void vec3_cross(float *out,const float *a,const float *b) {
  float tmp[3]={a[1]*b[2]-a[2]*b[1],
                a[2]*b[0]-a[0]*b[2],
                a[0]*b[1]-a[1]*b[0]};


  memcpy(out,tmp,sizeof(tmp));
}

float vec3_dot(const float *a,const float *b) {
  return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}

float vec3_length(const float *v) {
  return sqrtf(vec3_dot(v,v));
}

void vec3_normal(float *out,const float *v) {
  float l=1.0f/vec3_length(v);
  // float l= InvSqrt(vec3_dot(v,v));
  out[0]=v[0]*l;
  out[1]=v[1]*l;
  out[2]=v[2]*l;
}

void vec3_add(float *out,const float *a,const float *b) {
  out[0]=a[0]+b[0];
  out[1]=a[1]+b[1];
  out[2]=a[2]+b[2];
}

void vec3_sub(float *out,const float *a,const float *b) {
  out[0]=a[0]-b[0];
  out[1]=a[1]-b[1];
  out[2]=a[2]-b[2];
}

void vec3_mulScalar(float *out,
                    const float *v,
                    const float s) {
  out[0]=v[0]*s;
  out[1]=v[1]*s;
  out[2]=v[2]*s;
}

//===mat4


void mat4_identity(float *out) {
  int i;

  for(i=0;i<16;i++) {
    out[i]=(i%5)?0.0f:1.0f;
  }
}

void mat4_transpose(float *out,float *m) {
  int i,j;
  for(i=0;i<4;i++){
    for(j=i+1;j<4;j++){
      float a = m[i*4+j];
      out[i*4+j] = m[j*4+i];
      out[j*4+i] = a;
    }
  }
}

void mat4_mul(float *out,const float *a,
                   const float *b) {
  int i,j,k;
  float tmp[16];

  for(i=0;i<4;i++) {
    for(j=0;j<4;j++){
      float x=0.0f;

      for(k=0;k<4;k++){
        x+=a[i*4+k]*b[k*4+j];
      }

      tmp[i*4+j]=x;
    }
  }

  memcpy(out,tmp,sizeof(tmp));
}

void mat4_mulVector(float *out,
                         const float *m,
                         const float *v) {
  int i,j;
  float tmp[4];

  for(i=0;i<4;i++) {
    tmp[i]=0.0f;

    for(j=0;j<4;j++){
      tmp[i]+=m[i*4+j]*v[j];
    }
  }

  memcpy(out,tmp,sizeof(tmp));
}

void mat4_inverse(float *out,const float *m) {
  int K,q,L;
  float m2[16],tmp[16];
  mat4_identity(tmp);

  memcpy(m2,m,sizeof(m2));

  for(K=0;K<4;++K) {
    float factor = m2[K*4+K];

    for(q=0;q<4;q++) {
      m2[K*4+q] /= factor;
      tmp[K*4+q] /= factor;
    }

    for(L=0;L<4;++L){
      if(K==L) {
        continue;
      }

      float coefficient = m2[L*4+K];

      for(q=0;q<4;q++) {
        m2[L*4+q] -= coefficient*m2[K*4+q];
        tmp[L*4+q] -= coefficient*tmp[K*4+q];
      }
    }
  }

  memcpy(out,tmp,sizeof(tmp));
}


void mat4_normal(float *out,const float *m) {
  float m2[16];
  mat4_inverse(m2,m);
  mat4_transpose(m2,m2);

  memcpy(out,m2,sizeof(m2));
}


void mat4_toMat3(float *out,const float *m) {
  int i;

  for(i=0;i<9;i++) {
    out[i] = m[(i / 3) * 4 + i % 3];
  }
}

void mat4_translate(float *out,const float *v, bool mul) {
  int i;
  float mat[16];
  mat4_identity(mat);

  for(i=0;i<3;i++) {
    mat[3+i*4]=v[i];
  }

  //
  if(mul) {
    mat4_mul(out,out,mat);
  } else {
    memcpy(out,mat,sizeof(mat));
  }
}

void mat4_scale(float *out,const float *v,bool mul) {
  int i;
  float mat[16];
  mat4_identity(mat);

  for(i=0;i<3;i++) {
    mat[i*5]=v[i];
  }

  //
  if(mul) {
    mat4_mul(out,out,mat);
  } else {
    memcpy(out,mat,sizeof(mat));
  }
}


void mat4_rotateX(float *out,float x, bool mul) {
  float mat[16];
  mat4_identity(mat);

  float c=cosf(x);
  float s=sinf(x);

  mat[5]=c;
  mat[6]=-s;
  mat[9]=s;
  mat[10]=c;

  //
  if(mul) {
    mat4_mul(out,out,mat);
  } else {
    memcpy(out,mat,sizeof(mat));
  }
}

void mat4_rotateY(float *out,float y,bool mul) {
  float mat[16];
  mat4_identity(mat);

  float c=cosf(y);
  float s=sinf(y);

  mat[0]=c;
  mat[2]=s;
  mat[8]=-s;
  mat[10]=c;

  //
  if(mul) {
    mat4_mul(out,out,mat);
  } else {
    memcpy(out,mat,sizeof(mat));
  }
}

void mat4_rotateZ(float *out,float z,bool mul) {
  float mat[16];
  mat4_identity(mat);

  float c=cosf(z);
  float s=sinf(z);

  mat[0]=c;
  mat[1]=-s;
  mat[4]=s;
  mat[5]=c;

  //
  if(mul) {
    mat4_mul(out,out,mat);
  } else {
    memcpy(out,mat,sizeof(mat));
  }
}

void mat4_rotateAxis(float *out,const float *axis,
                     float angle,bool norm,bool mul) {
  float axis2[3];
  float mat[16];
  float u,v,w,L,u2,v2,w2;

  mat4_identity(mat);

  if(norm) {
    vec3_normal(axis2,axis);
  } else {
    axis2[0]=axis[0];
    axis2[1]=axis[1];
    axis2[2]=axis[2];
  }

  u=axis2[0];
  v=axis2[1];
  w=axis2[2];

  L=(u*u+v*v+w*w);

  u2=axis2[0]*axis2[0];
  v2=axis2[1]*axis2[1];
  w2=axis2[2]*axis2[2];

  mat[0]=(u2+(v2+w2)*cosf(angle))/L;
  mat[1]=(u*v*(1.0f-cosf(angle))-w*sqrtf(L)*sinf(angle))/L;
  mat[2]=(u*w*(1.0f-cosf(angle))+v*sqrtf(L)*sinf(angle))/L;

  mat[4]=(u*v*(1.0f-cosf(angle))+w*sqrtf(L)*sinf(angle))/L;
  mat[5]=(v2+(u2+w2)*cosf(angle))/L;
  mat[6]=(v*w*(1.0f-cosf(angle))-u*sqrtf(L)*sinf(angle))/L;

  mat[8]=(u*w*(1.0f-cosf(angle))-v*sqrtf(L)*sinf(angle))/L;
  mat[9]=(v*w*(1.0f-cosf(angle))+u*sqrtf(L)*sinf(angle))/L;
  mat[10]=(w2+(u2+v2)*cosf(angle))/L;

  //
  if(mul) {
    mat4_mul(out,out,mat);
  } else {
    memcpy(out,mat,sizeof(mat));
  }
}

//===

void mat3_mulVector(float *out,const float *m,const float *v) {
  int i,j;
  float tmp[3];

  for(i=0;i<3;i++) {
    tmp[i]=0.0f;

    for(j=0;j<3;j++){
      tmp[i]+=m[i*3+j]*v[j];
    }
  }

  memcpy(out,tmp,sizeof(tmp));
}

//===

void reflect(float *out,const float *II,const float *N) {
  //I - 2.0 * dot(N, I) * N.

  float a=2.0f*vec3_dot(N,II);
  out[0]=II[0]-a*N[0];
  out[1]=II[1]-a*N[1];
  out[2]=II[2]-a*N[2];

}

float minOf(float x, float y) {
  return (x>y)?y:x;
}

float maxOf(float x, float y) {
  return (x>y)?x:y;
}

float clamp(float x, float minVal, float maxVal) {
  return minOf(maxOf(x, minVal), maxVal);
}

float mix(float x, float y, float a) {
  return x*(1-a)+y*a;
}
float saturate(float x) {
  return maxOf(0.0f,minOf(1.0f,x));
}

float smoothStep(float a,float b,float x) {
  float t = saturate((x-a)/(b-a));
  return t*t*(3.0f-2.0f*t);
}
float fract(float x) {
  return x - floorf(x);
}
float lerp(float f0,float f1,float u) {
  return (1.0f-u)*f0+u*f1;
}

void keyframer(float frame, const float *positions,
               const float *values, int valuesNum, int vecLen,
               float repeatGap,bool smooth, float *out) {

  int i,j;
  float frame2;
  float pos1,pos2;

  //
  if(valuesNum==0) {
    memset(out,0,sizeof(float)*vecLen);
    return;
  }

  //
  if(valuesNum==1) {
    //return values from only keyframe
    memcpy(out,values,sizeof(float)*vecLen);
    return;
  }


  //
  if(repeatGap!=0.0f) {
    //todo
    frame2=(float)fmod((double)frame,(double)positions[valuesNum-1]);
  } else {
    //when frame before beginning
    if(frame <= positions[0]) {
      //return values from first keyframe
      memcpy(out,values,sizeof(float)*vecLen);
      return;
    }

    //when frame past end
    if(frame >= positions[valuesNum-1]) {
      //return values from last keyframe
      memcpy(out,&values[(valuesNum-1)*vecLen],sizeof(float)*vecLen);
      return;
    }

    frame2=frame;
  }


  //
  i=0;

  while(true) {
    pos1=positions[i];
    pos2=positions[i+1];

    if(frame2>=pos1 && frame2<=pos2) {
      float v=(frame2-pos1)/(pos2-pos1);

      for(j=0;j<vecLen;j++) {
        float val1=values[i*vecLen+j];
        float val2=values[(i+1)*vecLen+j];

        if(smooth) {
          out[j]=lerp(val1,val2,smoothStep(0.0f,1.0f,v));
        } else {
          out[j]=lerp(val1,val2,v);
        }
      }

      return;
    }

    i++;
  }
}


//===intersections


bool intersectRayPlane(const float *P,const float *V,
                       const float *plane,float *t) {
  //ax+by+cz+d=0
  float d, NPd, NV;
  const float *N;

  //
  d=plane[3];
  N=plane;

  //
  NPd=vec3_dot(N,P)+d;

  //origin below plane
  //and on
  if(NPd <= 0.0f) {
    return false;
  }

  //
  NV=vec3_dot(N,V);

  //vector parallel or pointing away from plane
  if(NV >= 0.0f) {
    return false;
  }


  //
  (*t)=-(NPd)/NV;

  //
  return true;
}


bool intersectRaySphere(const float *P,const float *V,
                        const float *C,float r,
                        float *t) {
  float M[3],b,d,c;

  //
  vec3_sub(M,P,C);
  c=vec3_dot(M,M)-r*r;

  //inside sphere
  if(c<0.0f) {
    return false;
  }

  //
  b=vec3_dot(V,M);

  //ray pointing away from sphere
  // when on or outside sphere (c>=0)
  if(b > 0.0f) {
    return false;
  }

  //
  d=b*b-c;

  //no real roots (ray pointing towards, but misses)
  if(d < 0.0f) {
    return false;
  }

  //
  (*t)=-b-sqrtf(d);

  //
  return true;
}


bool intersectRayTriangle(const float *P,const float *V,
                          const float *Q0,const float *Q1,
                          const float *Q2,
                          const float *plane,
                          float *t,float *colPt,
                          float *bcOut) {
  //ax+by+cz+d=0
  float d, NPd, NV;
  const float *N;
  float u0,u1,u2,v0,v1,v2;

  //
  d=plane[3];
  N=plane;

  //
  NPd=vec3_dot(N,P)+d;

  //origin below plane
  if(NPd <= 0.0f) {
    return false;
  }

  //
  NV=vec3_dot(N,V);

  //vector parallel or pointing away from plane
  if(NV >= 0.0f) {
    return false;
  }


  //
  (*t)=-NPd/NV;

  //
  vec3_mulScalar(colPt,V,(*t));
  vec3_add(colPt,colPt,P);

  //rest from '3D Math Primer For Graphics And Game Development'

  //
  if(fabs((double)N[0]) > fabs((double)N[1])) {
    if(fabs((double)N[0]) > fabs((double)N[2])) {
      u0 = colPt[1]-Q0[1];
      u1 = Q1[1]-Q0[1];
      u2 = Q2[1]-Q0[1];
      v0 = colPt[2]-Q0[2];
      v1 = Q1[2]-Q0[2];
      v2 = Q2[2]-Q0[2];
    } else {
      u0 = colPt[0]-Q0[0];
      u1 = Q1[0]-Q0[0];
      u2 = Q2[0]-Q0[0];
      v0 = colPt[1]-Q0[1];
      v1 = Q1[1]-Q0[1];
      v2 = Q2[1]-Q0[1];
    }
  } else {
    if(fabs((double)N[1]) > fabs((double)N[2])) {
      u0 = colPt[0]-Q0[0];
      u1 = Q1[0]-Q0[0];
      u2 = Q2[0]-Q0[0];
      v0 = colPt[2]-Q0[2];
      v1 = Q1[2]-Q0[2];
      v2 = Q2[2]-Q0[2];
    } else {
      u0 = colPt[0]-Q0[0];
      u1 = Q1[0]-Q0[0];
      u2 = Q2[0]-Q0[0];
      v0 = colPt[1]-Q0[1];
      v1 = Q1[1]-Q0[1];
      v2 = Q2[1]-Q0[1];
    }
  }

  //

  // Compute denominator, check for invalid
  float temp = u1*v2-v1*u2;

  if(!(temp != 0.0f)) {
    return false;
  }

  temp = 1.0f / temp;

  // Compute barycentric coords, checking for out-of-range
  // at each step
  float alpha = (u0 * v2-v0 * u2) * temp;

  if (!(alpha >= 0.0f)) {
    return false;
  }

  float beta = (u1 * v0-v1 * u0) * temp;

  if (!(beta >= 0.0f)) {
    return false;
  }

  float gamma = 1.0f-alpha-beta;

  if (!(gamma >= 0.0f)) {
    return false;
  }

  bcOut[0]=alpha;
  bcOut[1]=beta;
  bcOut[2]=gamma;
  //
  return true;
}


bool intersectRayBox(const float *P,const float *invV,
                     const float *boxMin,const float *boxMax,
                     float *h0,float *h1) {

  //from libdimension
  // This is actually correct, even though it appears not to handle edge cases
  // (line.n.{x,y,z} == 0).  It works because the infinities that result from
  // dividing by zero will still behave correctly in the comparisons.  Lines
  // which are parallel to an axis and outside the box will have tmin == inf
  // or tmax == -inf, while lines inside the box will have tmin and tmax
  // unchanged.
  float t=INFINITY;
  float tx1 = (boxMin[0] - P[0])*invV[0];
  float tx2 = (boxMax[0] - P[0])*invV[0];

  float tmin = minOf(tx1, tx2);
  float tmax = maxOf(tx1, tx2);

  float ty1 = (boxMin[1] - P[1])*invV[1];
  float ty2 = (boxMax[1] - P[1])*invV[1];

  tmin = maxOf(tmin, minOf(ty1, ty2));
  tmax = minOf(tmax, maxOf(ty1, ty2));

  float tz1 = (boxMin[2] - P[2])*invV[2];
  float tz2 = (boxMax[2] - P[2])*invV[2];

  tmin = maxOf(tmin, minOf(tz1, tz2));
  tmax = minOf(tmax, maxOf(tz1, tz2));
  *h0=maxOf(0.0f,tmin);
  *h1=tmax;
  return tmax >= maxOf(0.0f, tmin) && tmin < t;
}


void planeFromPointNormal(const float *pt,
                          const float *nor,
                          bool norm,
                          float *plane) {

  //ax+by+cz+d=0

  plane[0]=nor[0]; //a
  plane[1]=nor[1]; //b
  plane[2]=nor[2]; //c

  if(norm) {
    vec3_normal(plane,plane);
  }

  plane[3]=-vec3_dot(pt,plane);//d
}

void planeFromTriangle(const float *pt0,const float *pt1,
                       const float *pt2,bool cw,
                       float *plane) {

  float E0[3],E1[3],N[3];
  vec3_sub(E0,pt1,pt0);
  vec3_sub(E1,pt2,pt0);
  vec3_cross(N,E0,E1);
  vec3_normal(N,N);

  if(!cw) {
    vec3_mulScalar(N,N,-1.0f);
  }

  planeFromPointNormal(pt0,N,false,plane);


}


void triangleBoundingBox(const float *pt0,
                         const float *pt1,
                         const float *pt2,
                         float *start,float *end) {
  int i;

  start[0]=end[0]=pt0[0];
  start[1]=end[1]=pt0[1];
  start[2]=end[2]=pt0[2];

  for(i=0;i<3;i++) {
    if(pt1[i] < start[i]) {
      start[i]=pt1[i];
    }

    if(pt1[i] > end[i]) {
      end[i]=pt1[i];
    }

    if(pt2[i] < start[i]) {
      start[i]=pt2[i];
    }

    if(pt2[i] > end[i]) {
      end[i]=pt2[i];
    }
  }
}


void sphereBoundingBox(const float *pos,float radius,
                       float *start,float *end) {
  int i;

  for(i=0;i<3;i++) {
    start[i]=pos[i]-radius;
    end[i]=pos[i]+radius;
  }
}

//===
int powi(int base, int exp) {
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


//===
void mat4_frustum(float *out,
                  float left,float right,
                  float bottom,float top,
                  float zNear,float zFar) {

  memset(out,0,sizeof(float)*16);

  float A=(right+left)/(right-left);
  float B=(top+bottom)/(top-bottom);
  float C=-(zFar+zNear)/(zFar-zNear);
  float D=-(2.0f*zFar*zNear)/(zFar-zNear);

  float x=(2.0f*zNear)/(right-left);
  float y=(2.0f*zNear)/(top-bottom);

  out[0]=x;
  out[2]=A;
  out[5]=y;
  out[6]=B;
  out[10]=C;
  out[11]=D;
  out[14]=-1.0f;
}

void mat4_infFrustum(float *out,
                     float left,float right,
                     float bottom,float top,
                     float zNear) {

  memset(out,0,sizeof(float)*16);

  float A=(right+left)/(right-left);
  float B=(top+bottom)/(top-bottom);
  float C=-1.0f;
  float D=-2.0f*zNear;

  float x=(2.0f*zNear)/(right-left);
  float y=(2.0f*zNear)/(top-bottom);

  out[0]=x;
  out[2]=A;
  out[5]=y;
  out[6]=B;
  out[10]=C;
  out[11]=D;
  out[14]=-1.0f;
}

void mat4_perspective_fovx(float *out,
                        float fovx,float aspect,
                        float znear,float zfar) {
  float right=tan(fovx/2.0f)*znear;
  float left=-right;
  float top=right/aspect;
  float bottom=-top;
  mat4_frustum(out,left,right,bottom,top,znear,zfar);
}

void mat4_perspective_fovy(float *out,
                        float fovy,float aspect,
                        float znear,float zfar) {

  float top=tan(fovy/2.0f)*znear;
  float bottom=-top;
  float right=top*aspect;
  float left=-right;
  mat4_frustum(out,left,right,bottom,top,znear,zfar);
}

void mat4_infPerspective_fovx(float *out,
                              float fovx,float aspect,
                              float znear) {
  float right=tan(fovx/2.0f)*znear;
  float left=-right;
  float top=right/aspect;
  float bottom=-top;
  mat4_infFrustum(out,left,right,bottom,top,znear);
}

void mat4_infPerspective_fovy(float *out,
                              float fovy,float aspect,
                              float znear) {

  float top=tan(fovy/2.0f)*znear;
  float bottom=-top;
  float right=top*aspect;
  float left=-right;
  mat4_infFrustum(out,left,right,bottom,top,znear);
}
void mat4_perspective(float *out,float fov,float aspect,
                      float znear,float zfar) {
  float yScale=1.0f/tanf(fov/2.0f);
  float xScale=yScale/aspect;
  float nearmfar=znear-zfar;

  memset(out,0,sizeof(float)*16);

  out[0]=xScale;
  out[5]=yScale;
  out[10]=(zfar+znear)/nearmfar;
  out[11]=2.0f*zfar*znear/nearmfar;
  out[14]=-1.0f;
}

void mat4_perspectiveInf(float *out,float fov,float aspect,float znear) {
  float yScale=1.0f/tanf(fov/2.0f);
  float xScale=yScale/aspect;

  memset(out,0,sizeof(float)*16);

  out[0]=xScale;
  out[5]=yScale;
  out[10]=-1.0f;
  out[11]=-2.0f*znear;
  out[14]=-1.0f;
}

void mat4_ortho(float *out,float left,float right,
                float bottom,float top,float nearVal,
                float farVal) {

  memset(out,0,sizeof(float)*16);

  float tx = -(right+left)/(right-left);
  float ty = -(top+bottom)/(top-bottom);
  float tz = -(farVal+nearVal)/(farVal-nearVal);

  out[0]=2.0f/(right-left);
  out[3]=tx;
  out[5]=2.0f/(top-bottom);
  out[7]=ty;
  out[10]=-2.0f/(nearVal-farVal);
  out[11]=tz;
  out[15]=1.0f;
}

void mat4_ortho2d(float *out,float left,float right,float bottom,float top) {
  mat4_ortho(out,left,right,bottom,top,-1.0f,1.0f);
}

void mat4_lookAt(float *out,
                 float px,float py,float pz,
                 float ax,float ay,float az,
                 float ux,float uy,float uz) {
  //untested

  float eye[3]={px,py,pz};
  float at[3]={ax,ay,az};
  float up[3]={ux,uy,uz};

  float zaxis[3],xaxis[3],yaxis[3];

  vec3_sub(zaxis,at,eye);
  vec3_normal(zaxis,zaxis);

  vec3_cross(xaxis,zaxis,up);
  vec3_normal(xaxis,xaxis);

  vec3_cross(yaxis,xaxis,zaxis);
  vec3_normal(yaxis,yaxis);

  out[0]=xaxis[0];
  out[1]=yaxis[0];
  out[2]=zaxis[0];
  out[3]=0.0f;

  out[4]=xaxis[1];
  out[5]=yaxis[1];
  out[6]=zaxis[1];
  out[7]=0.0f;


  out[8]=xaxis[2];
  out[9]=yaxis[2];
  out[10]=zaxis[2];
  out[11]=0.0f;

  out[12]=-vec3_dot(xaxis,eye);
  out[13]=-vec3_dot(yaxis,eye);
  out[14]=-vec3_dot(zaxis,eye);
  out[15]=1.0f;

  //wrong handed? row major?
}

//======================

void mat4_translateXYZ(float *out,float x,float y, float z,bool mul) {
  float v[3]={x,y,z};
  mat4_translate(out,v,mul);
}

void mat4_scaleXYZ(float *out,float x,float y, float z,bool mul) {
  float v[3]={x,y,z};
  mat4_scale(out,v,mul);
}
