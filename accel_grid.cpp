#include<fstream>
#include<iostream>
#include<cmath>
#include<algorithm>
#include<stack>
#include<vector>
#include<sstream>
#include<string>
#include<climits>
#include "Transform.h"
using namespace std;
#include "variables.h"

void generate_grid(){

  // First step is to get an estimate of grid spacing and the dimensions of the bounding box
  object obj;

  //First iteration through objects to get bounding box dimensions
  double min_z=INT_MAX,max_z=INT_MIN,min_x=INT_MAX,min_y=INT_MAX,max_x=INT_MIN,max_y=INT_MIN;
  for(int i=0;i<numobjects;i++){
    obj = objects[i];
    if(obj.type==SPHERE)continue;
    vec3 vT = vec3(obj.transform*vec4(obj.v1,1));
    min_z=min(min_z,(double)vT[2]);
    max_z=max(max_z,(double)vT[2]);
    min_y=min(min_y,(double)vT[1]);
    min_x=min(min_x,(double)vT[0]);
    max_x=max(max_x,(double)vT[0]);
    max_y=max(max_y,(double)vT[1]);
    
    vT = vec3(obj.transform*vec4(obj.v2,1));
    min_z=min(min_z,(double)vT[2]);
    max_z=max(max_z,(double)vT[2]);
    min_y=min(min_y,(double)vT[1]);
    min_x=min(min_x,(double)vT[0]);
    max_x=max(max_x,(double)vT[0]);
    max_y=max(max_y,(double)vT[1]);
    
    vT = vec3(obj.transform*vec4(obj.v3,1));
    min_z=min(min_z,(double)vT[2]);
    max_z=max(max_z,(double)vT[2]);
    min_y=min(min_y,(double)vT[1]);
    min_x=min(min_x,(double)vT[0]);
    max_x=max(max_x,(double)vT[0]);
    max_y=max(max_y,(double)vT[1]);
    
  }
  //Add origin to boundry calculation
   min_z=min(min_z,0.);
   max_z=max(max_z,0.);
   min_y=min(min_y,0.);
   min_x=min(min_x,0.);
   max_x=max(max_x,0.);
   max_y=max(max_y,0.);
   
  //Check if min==max!!
  if(min_x==max_x){
    min_x--;
    max_x++;
  }
  if(min_y==max_y){
    min_y--;
    max_y++;
  }
  if(min_z==0){
    min_z--;
  }

 
  //Find optimal resolution
  int Volume = (max_x-min_x)*(max_y-min_y)*(max_z-min_z);
  Nx = (max_x-min_x);//*cbrt(grid_resolution_parameter*numobjects/Volume);
  Ny = (max_y-min_y);//*cbrt(grid_resolution_parameter*numobjects/Volume);
  Nz = (max_z-min_z);//*cbrt(grid_resolution_parameter*numobjects/Volume);
  
  //make even??
  Nx+=Nx%2;
  Ny+=Ny%2;
  Nz+=Nz%2;
  
  gdx= (max_x-min_x)/Nx;
  gdy= (max_y-min_y)/Ny;
  gdz= (max_z-min_z)/Nz;

  Nx+=2;
  Ny+=2;
  Nz+=2;

  // Define offsets for use in main function
  offset_x=abs(min_x);
  offset_y=abs(min_y);
  offset_z=abs(min_z);

  
  grid = vector< vector<int> >(Nx*Ny*Nz,vector<int>());

  for(int l=0;l<numobjects;l++){
    obj = objects[l];
    if(obj.type==SPHERE)continue;
    vec3 vt1 = vec3(obj.transform*vec4(obj.v1,1));
    vec3 vt2 = vec3(obj.transform*vec4(obj.v2,1));
    vec3 vt3 = vec3(obj.transform*vec4(obj.v3,1));
    
    double minx=min(vt1[0],min(vt2[0],vt3[0])),maxx=max(vt1[0],max(vt2[0],vt3[0])),miny=min(vt1[1],min(vt2[1],vt3[1])),maxy=max(vt1[1],max(vt2[1],vt3[1])),maxz=max(vt1[2],max(vt2[2],vt3[2])),minz=min(vt1[2],min(vt2[2],vt3[2]));

    int minNx = (minx+offset_x)/gdx,
      maxNx = (maxx+offset_x)/gdx,
      minNy = (miny+offset_y)/gdy,
      maxNy = (maxy+offset_y)/gdy,
      minNz = (minz+offset_z)/gdz,
      maxNz = (maxz+offset_z)/gdz;
    
    //cout<<minNx<<' '<<maxNx<<' '<<minNy<<' '<<maxNy<<' '<<minNz<<' '<<maxNz<<endl;
    
     for(int i=minNx;i<=maxNx;i++)
      for(int j=minNy;j<=maxNy;j++)
	for(int k=minNz;k<=maxNz;k++){
	  /*cout<<"****************\n";
	  cout<<i<<' '<<j<<' '<<k<<endl;
	  cout<<k*Nx*Ny+j*Nx+i<<endl;
	  cout<<l<<endl;
	  cout<<"****************\n";*/
	  grid[k*Nx*Ny+j*Nx+i].push_back(l);
	}
    
     /*cout<<Nx<<' '<<Ny<<' '<<Nz<<endl;
       cout<<gdx<<' '<<gdy<<' '<<gdz<<endl;*/
  }
  
}
