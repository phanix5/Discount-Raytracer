// Transform.cpp: implementation of the Transform class.

// Note: when you construct a matrix using mat4() or mat3(), it will be COLUMN-MAJOR
// Keep this in mind in readfile.cpp and display.cpp
// See FAQ for more details or if you're having problems.
#include <string>
#include "Transform.h"


mat3 Transform::rotate(const float degrees, const vec3& axis) 
{
	mat3 ret;
  	mat3 id = mat3(1.0);
	float c= cos(degrees*pi/180);
	float s= sin(degrees*pi/180);
	mat3 cosmat=mat3(c,c,c,c,c,c,c,c,c);
	mat3 cosminmat=mat3(1-c,1-c,1-c,1-c,1-c,1-c,1-c,1-c,1-c);
	mat3 sinmat=mat3(s,s,s,s,s,s,s,s,s);
	mat3 A=mat3(0,axis[2],-axis[1],-axis[2],0,axis[0],axis[1],-axis[0],0);
	return glm::matrixCompMult(cosmat,id)+glm::matrixCompMult(cosminmat,glm::outerProduct(axis,axis))+glm::matrixCompMult(sinmat,A);

}

void Transform::left(float degrees, vec3& eye, vec3& up) 
{
  
  eye=Transform::rotate(degrees,up)*eye;
  
}

void Transform::up(float degrees, vec3& eye, vec3& up) 
{
  
  	vec3 axis=glm::cross(eye,up);
	axis=axis/glm::length(axis);
	eye=Transform::rotate(degrees,axis)*eye;
	up=Transform::rotate(degrees,axis)*up; 

  
}

mat4 Transform::lookAt(const vec3 &eye, const vec3 &center, const vec3 &up) 
{
 
  vec3 w=(eye-center)/glm::length(eye-center);
	vec3 u=glm::cross(up,w);
	u=u/glm::length(u);
	vec3 v=glm::cross(w,u);
	mat3 rot=mat3(u,v,w);
	rot=glm::transpose(rot);
	mat4 rot4=mat4(rot);
	mat4 trans=mat4(1.0);
	trans[3][0]=-eye[0];
	trans[3][1]=-eye[1];
	trans[3][2]=-eye[2];
	mat4 res=rot4*trans;

  
  return res;
}

mat4 Transform::perspective(float fovy, float aspect, float zNear, float zFar)
{
  mat4 ret=mat4(1.0);
  
  double d = 1/tan(fovy*pi/360);
  float A = -(zFar+zNear)/(zFar-zNear), B=-2*zFar*zNear/(zFar-zNear);
  ret[0][0]=d/aspect;
  ret[1][1]=d;
  ret[2][2]=A;
  ret[3][3]=0;
  ret[3][2]=B;
  ret[2][3]=-1;
  // New, to implement the perspective transform as well.  
  return ret;
}

mat4 Transform::scale(const float &sx, const float &sy, const float &sz) 
{
  mat4 ret=mat4(1.0);
  
  ret[0][0]=sx;
  ret[1][1]=sy;
  ret[2][2]=sz;
  
  return ret;
}

mat4 Transform::translate(const float &tx, const float &ty, const float &tz) 
{
  mat4 ret=mat4(1.0);
  
  ret[3][0]=tx;
  ret[3][1]=ty;
  ret[3][2]=tz;

  
  return ret;
}

// To normalize the up direction and construct a coordinate frame.  

vec3 Transform::upvector(const vec3 &up, const vec3 & zvec) 
{
  vec3 x = glm::cross(up,zvec); 
  vec3 y = glm::cross(zvec,x); 
  vec3 ret = glm::normalize(y); 
  return ret; 
}


Transform::Transform()
{

}

Transform::~Transform()
{

}
