#include<fstream>
#include<iostream>
#include<cmath>
#include<stack>
#include<vector>
#include<sstream>
#include<string>
#include<time.h>
#include "Transform.h"
#define MAINPROGRAM
using namespace std;
#include "variables.h" 
#include "readfile.h" // prototypes for readfile.cpp
#include "STLParser.h"
#include "accel_grid.h"


void printv(vec3 a){
  cout<<a[0]<<' '<<a[1]<<' '<<a[2]<<endl;
}

struct normal{
  double x,y,z;
  normal(){x=y=z=0;}
  normal(double a,double b,double c){x=a,y=b,z=c;}
  normal operator + (normal v){return normal(x+v.x,y+v.y,z+v.z);}
  normal operator - (normal v){return normal(x-v.x,y-v.y,z-v.z);}
};


struct ray{
  vec3 o;    //Origin
  vec3 d;    //Direction
  double t_min,t_max;
  ray(){o=d=vec3(0);}
  ray(vec3 i,vec3 j) {o=i;d=j;}
  ray(vec3 i,vec3 j,double tmin,double tmax) {o=i;d=j;t_min=tmin;t_max=tmax;}
  vec3 getPoint(double t){return o+(float)t*d;} 
};

struct Color {
  double r,g,b;
  Color(){r=g=b=0;}
  Color(double i,double j,double k){r=i,g=j,b=k;}

  void black(){r=0,g=0,b=0;}
  Color operator * (double d){return Color(r*d,g*d,b*d);}
  Color operator + (Color d){return Color(r+d.r,g+d.g,b+d.b);}
  void add(double *col){r+=col[0];g+=col[1];b+=col[2];}
  void add(vec3 col){r+=col[0];g+=col[1];b+=col[2];}
  vec3 Vec(){return vec3(r,g,b);}
};

struct sample{
public:
  int x,y;
  sample(){x=y=0;}
};

struct sampler{
  int width,height;
  sampler(int w,int h){width=w,height=h;}
  bool getSample(sample &s){
    if(s.x == width){
      s.x=0;
      s.y+=1;
      if(s.y==height)return false;
      return true;
    }
    return true;
  }
  void nextSample(sample &s){
    s.x+=1;
  }
  
};
struct LocalGeo{
  vec3 point,normal,gpoint,gnormal;
  int index;
  LocalGeo(){point=vec3(0);normal=vec3(0);}
  LocalGeo(vec3 p,vec3 n){point=p;normal=n;}
};
struct Camera{
  double z,width,height,pixelWidth;
  Camera(vec3 &eye,vec3 &center){
    z=glm::length(eye-center);
    height=2*z*tan(fovy*pi/360);
    width=height*w/h;
    fovx=360*atan(width*0.5/z)/pi;
    rayOffset=0.5*width/w;
    pixelWidth=width/w;
  }
public:
  ray generateRay(sample &s){
    double alpha =(s.x*width/w+0.5*pixelWidth-width*0.5);
    double beta = (height*0.5-s.y*height/h-0.5*pixelWidth);
    vec3 dir(alpha,beta,-z);
    double t_min=sqrt(alpha*alpha+beta*beta+z*z);
    dir=glm::normalize(dir);
    return ray(vec3(0,0,0),dir,0.1*t_min,100*t_min);
  }
};

class Primitive{
public:
  
  static bool intersectS(ray &r,double &thit,primitive &prim,LocalGeo &localgeo){
    //Transform the ray
     vec4 newOrigin=prim.invTransform*vec4(r.o,1);
    //homogenize
    newOrigin=newOrigin/newOrigin[3];
    vec3 newDir=vec3(prim.invTransform*vec4(r.d,0));
    ray tRay=ray(vec3(newOrigin),glm::normalize(newDir));
   
    vec3 oc = tRay.o-prim.center;
    double b= 2*glm::dot(oc,tRay.d);
    double c=glm::dot(oc,oc)-prim.radius*prim.radius;
    double disc=b*b-4*c;
    if(disc<0)return false;
    disc=sqrt(disc);
    double t0=-b-disc;
    double t1=-b+disc;

    double t=(t0<t1)?t0/2:t1/2;
    if(t<=0)return false;
    vec3 lpoint=(tRay.getPoint(t));
    vec4 gpoint4=prim.transform*vec4(lpoint,1);
    gpoint4=gpoint4/gpoint4[3];
    vec3 gpoint(gpoint4);
    vec3 tmp=gpoint-r.o;
    thit=glm::length(tmp);
   
    if(thit<r.t_min || thit>r.t_max)return false;
    localgeo.point=lpoint;
    localgeo.normal=glm::normalize(lpoint-prim.center);
    localgeo.gpoint=gpoint;
    localgeo.gnormal=glm::normalize(vec3(glm::transpose(prim.invTransform)*vec4(localgeo.normal,0)));
    
    return true;
  }
  static bool intersectT(ray &r,double &thit,primitive &prim,LocalGeo &localgeo){
    
    //Transform the ray
    vec4 newOrigin=prim.invTransform*vec4(r.o,1);
    //homogenize
    newOrigin=newOrigin/newOrigin[3];
    vec3 newDir=vec3(prim.invTransform*vec4(r.d,0));
    ray tRay=ray(vec3(newOrigin),glm::normalize(newDir));
    
    //first find normal to the triangle
    vec3 normal= glm::cross(prim.v3-prim.v1,prim.v2-prim.v1);
    normal=glm::normalize(normal);
   
    //Correct the direction of the normal
    if(glm::dot(normal,tRay.d)>0)normal=vec3(0)-normal;
    
    //check if ray parallel to plane, return false if true
    if(glm::dot(tRay.d,normal)==0)return false;

    //Find the intersection point
    double t = (glm::dot(prim.v1,normal)-glm::dot(tRay.o,normal))/glm::dot(tRay.d,normal);
    if(t<=0)return false;
    
    // Check limits for t
    vec3 lpoint=(tRay.getPoint(t));
    vec4 gpoint4=prim.transform*vec4(lpoint,1);
    gpoint4=gpoint4/gpoint4[3];
    vec3 gpoint(gpoint4);
    vec3 tmp=gpoint-r.o;
    thit=glm::length(tmp);
    if(thit<r.t_min || thit>r.t_max)return false;
    
    //Finally check if intersection within triangle
    vec3 v0=prim.v3-prim.v1;
    vec3 v1=prim.v2-prim.v1;
    vec3 v2=tRay.getPoint(t)-prim.v1;

    double u,v;

    u=(glm::dot(v1,v1)*glm::dot(v2,v0)-glm::dot(v1,v0)*glm::dot(v2,v1))/
      (glm::dot(v0,v0)*glm::dot(v1,v1)-glm::dot(v0,v1)*glm::dot(v1,v0));
    v=(glm::dot(v0,v0)*glm::dot(v2,v1)-glm::dot(v1,v0)*glm::dot(v2,v0))/
      (glm::dot(v0,v0)*glm::dot(v1,v1)-glm::dot(v0,v1)*glm::dot(v1,v0));

    if(u>=0 && v>=0 && u<=1 && v<=1 && u+v<=1){
      localgeo.point=lpoint;
      localgeo.normal=normal;
      localgeo.gpoint=gpoint;
      localgeo.gnormal=glm::normalize(vec3(glm::transpose(prim.invTransform)*vec4(localgeo.normal,0)));
      return true;
    }
    return false;

  }

  static bool intersect(ray &r,double &thit,primitive &prim,LocalGeo &localgeo){
    if(prim.type==TRIANGLE)return intersectT(r,thit,prim,localgeo);
    return intersectS(r,thit,prim,localgeo);
  }
};

class Shape{
public:
  static bool intersect(ray& r,double &thit, LocalGeo& localgeo){

    vector <int> index(3,0);
    index[0]=(r.o[0]+offset_x)/gdx;
    index[1]=(r.o[1]+offset_y)/gdy;
    index[2]=(r.o[2]+offset_z)/gdz;
    vector <int> index_offset(3,1);
    if(r.d[0]<0)index_offset[0]=-1;
    if(r.d[1]<0)index_offset[1]=-1;
    if(r.d[2]<0)index_offset[2]=-1;
    vector <int> cell_size = index;
    if(r.d[0]>0)cell_size[0]++;
    if(r.d[1]>0)cell_size[1]++;
    if(r.d[2]>0)cell_size[2]++;
    vector <double> t0(3,0);
    t0[0]=abs(cell_size[0]*gdx-r.o[0]-offset_x)/abs(r.d[0]+gdx*1e-9);
    t0[1]=abs(cell_size[1]*gdy-r.o[1]-offset_y)/abs(r.d[1]+gdy*1e-9);
    t0[2]=abs(cell_size[2]*gdz-r.o[2]-offset_z)/abs(r.d[2]+gdz*1e-9);
    vector <double> dt(3,0);
    dt[0]=abs(gdx/(r.d[0]+gdx*1e-9));
    dt[1]=abs(gdy/(r.d[1]+gdy*1e-9));
    dt[2]=abs(gdz/(r.d[2]+gdz*1e-9));

    /*cout<<"**********************\n";
    cout<<r.o[0]<<' '<<r.o[1]<<' '<<r.o[2]<<endl;
    
    
    
    cout<<"**********************\n";
    if(r.d[1]<0&& r.d[0]<0){
      cout<<r.d[0]<<' '<<r.d[1]<<' '<<r.d[2]<<endl;
      cout<<index[0]<<' '<<index[1]<<' '<<index[2]<<endl;
      cout<<t0[0]<<' '<<t0[1]<<' '<<t0[2]<<endl;
      cout<<dt[0]<<' '<<dt[1]<<' '<<dt[2]<<endl;
      cout<<cell_size[2]<<' '<<gdz<<' '<< r.o[2]<<' '<<offset_z<<endl;
      }*/

    
    
    double t_x=t0[0],t_y=t0[1],t_z=t0[2],t=0;

    if(t0[0]==0 && r.d[0]<0)index[0]--,t_x=dt[0];
    if(t0[1]==0 && r.d[1]<0)index[1]--,t_y=dt[1];
    if(t0[2]==0 && r.d[2]<0)index[2]--,t_z=dt[2];
    
    while(index[0]>=0 && index[0]<Nx && index[1]>=0 && index[1]<Ny && index[2]>=0 && index[2]<Nz){

      vector <int> prim_list = grid[index[2]*Ny*Nx + index[1]*Nx+index[0]];
      bool hit=false;
      thit=r.t_max;
      //if(r.d[1]<0 && r.d[0]<0)cout<<index[0]<<' '<<index[1]<<' '<<index[2]<<endl;
      for(int i=0;i<prim_list.size();i++)
	{
	  double a;
	  LocalGeo tempgeo;
	  int ind = prim_list[i];
	  if(Primitive::intersect(r,a,primitives[ind],tempgeo)){
	    if(a<thit){
	      thit=a;
	      localgeo=tempgeo;
	      localgeo.index=ind;
	      hit=true;
	    }
	  }
	}
     

      

      if(t_x<t_y && t_x<t_z){
	t=t_x;
	t_x+=dt[0];
	index[0]+=index_offset[0];
      }
      else if(t_y<t_x && t_y<t_z){
	t=t_y;
	t_y+=dt[1];
	index[1]+=index_offset[1];
      }
      else{
	t=t_z;
	t_z+=dt[2];
	index[2]+=index_offset[2];
      }
      /*cout<<r.d[0]<<' '<<r.d[1]<<' '<<r.d[2]<<endl;*/
      if(hit && thit <= t)return true;
     
    }
    return false;
  }
};

class Light{
public:
  static ray generateLightRay(LocalGeo &localgeo,int Lindex){
    vec3 gpoint=localgeo.gpoint;
    
    vec3 gnormal=localgeo.gnormal;

    vec3 gOrigin=gpoint+(float)0.5*rayOffset*gnormal;
    if(lightposn[Lindex][3]==0){      //directional
      vec3 gDir=glm::normalize(vec3(lightransf[Lindex]));
      return ray(gOrigin,gDir,0,1e6);
    }
    vec3 gDir=glm::normalize(vec3(lightransf[Lindex])-gOrigin);
    return ray(gOrigin,gDir,0,glm::length(vec3(lightransf[Lindex])-gOrigin));
  }
  static vec3 shading(ray lray,LocalGeo &localgeo,int Lindex,int isPoint){

    vec3 N=localgeo.gnormal;
    vec3 L =lray.d;
    double r=glm::length(vec3(lightransf[Lindex])-lray.o);
    double *d=primitives[localgeo.index].diffuse;
    vec3 D = vec3(d[0],d[1],d[2]);
    d=primitives[localgeo.index].specular;
    vec3 S = vec3(d[0],d[1],d[2]);
    vec3 E = glm::normalize(globalEye - localgeo.gpoint);
    vec3 H = glm::normalize(E+L);
    float atten=1;
    double s=primitives[localgeo.index].shininess;
    if(isPoint)atten=1/(attenuation[0]+r*attenuation[1]+r*r*attenuation[2]);
    float NdotL = glm::dot(N,L);
    if(NdotL<0)NdotL=0;
    float RdotE = glm::dot(H,N);
    if(RdotE<0)RdotE=0;
    return atten*vec3(lightcolor[Lindex])*(NdotL*D+(float)pow(RdotE,s)*S);
    
  }

};

class RayTracer{
public:
  static void trace(ray &r,int depth,Color& color){
    color.black();    //Initilize the color primitive
    if(depth>maxdepth){
      color.black();
      return;
    }
    double thit=0;
    LocalGeo localgeo(vec3(0),vec3(0));
    if(!Shape::intersect(r,thit,localgeo)){
      color.black();
      return;
    }
    
    int index=localgeo.index;
    color.add(primitives[index].ambient);      // Ambient term
    color.add(primitives[index].emission);      // Emissive term


    //If intersecting then analyze lights
    for(int i=0;i<numused;i++){
      ray lray = Light::generateLightRay(localgeo,i);
      LocalGeo dummy;
      if(!Shape::intersect(lray,thit,dummy)){
	color.add(Light::shading(lray,localgeo,i,lightposn[i][3]));
      }
      }
    // Reflections
    // does not work with grid structure
    double *spec=primitives[localgeo.index].specular;
    if(!(spec[0]==0 && spec[1]==0 && spec[2]==0)){
      //setup reflected ray
      vec3 gN=localgeo.gnormal;
      vec3 gP=localgeo.gpoint;
      r.d=glm::normalize(r.d-2.0f*glm::dot(gN,r.d)*gN);
      r.o=gP+rayOffset*gN;
      r.t_min=0.01*r.t_min;
      r.t_max=100*r.t_max;
      Color rcolor;
      vec3 tmp = globalEye;
      globalEye = r.o;
      trace(r,depth+1,rcolor);
      globalEye = tmp;
      color.add(vec3(spec[0],spec[1],spec[2])*rcolor.Vec());
      }
  }

};
class Film{
public:
  uint8_t *pixels;
  Color **pixel_col;
  Film(int w,int h){
    pixels=new uint8_t[3*w*h];
    pixel_col=new Color*[h];
    for(int i=0;i<h;i++)pixel_col[i]=new Color[w];
  }
  void commitColor(sample &s,Color color){
    int ind=w*s.y+s.x;
    int col=color.r*255;
    uint8_t r=(uint8_t)col;
    col=color.g*255;
    uint8_t g=(uint8_t)col;
    col=color.b*255;
    uint8_t b=(uint8_t)col;
    pixels[ind]=r;
    pixels[ind+1]=g;
    pixels[ind+2]=b;
    pixel_col[s.y][s.x]=color;
  }
  
  void writeImageBMP() {
	  unsigned int headers[13];
	  FILE * outfile;
	  int extrabytes;
	  int paddedsize;
	  int x; int y; int n;
	  int red, green, blue;

	  extrabytes = 4 - ((w * 3) % 4);                 // How many bytes of padding to add to each
														  // horizontal line - the size of which must
														  // be a multiple of 4 bytes.
	  if (extrabytes == 4)
		  extrabytes = 0;

	  paddedsize = ((w * 3) + extrabytes) * h;
	  // Headers...
	  // Note that the "BM" identifier in bytes 0 and 1 is NOT included in these "headers".

	  headers[0] = paddedsize + 54;      // bfSize (whole file size)
	  headers[1] = 0;                    // bfReserved (both)
	  headers[2] = 54;                   // bfOffbits
	  headers[3] = 40;                   // biSize
	  headers[4] = w;  // biWidth
	  headers[5] = h; // biHeight

	  headers[7] = 0;                    // biCompression
	  headers[8] = paddedsize;           // biSizeImage
	  headers[9] = 0;                    // biXPelsPerMeter
	  headers[10] = 0;                    // biYPelsPerMeter
	  headers[11] = 0;                    // biClrUsed
	  headers[12] = 0;                    // biClrImportant

	  outfile = fopen("raytrace.bmp", "wb");
	  //
	  // Headers begin...
	  // When printing ints and shorts, we write out 1 character at a time to avoid endian issues.
	  //

	  fprintf(outfile, "BM");

	  for (n = 0; n <= 5; n++)
	  {
		  fprintf(outfile, "%c", headers[n] & 0x000000FF);
		  fprintf(outfile, "%c", (headers[n] & 0x0000FF00) >> 8);
		  fprintf(outfile, "%c", (headers[n] & 0x00FF0000) >> 16);
		  fprintf(outfile, "%c", (headers[n] & (unsigned int)0xFF000000) >> 24);
	  }

	  // These next 4 characters are for the biPlanes and biBitCount fields.

	  fprintf(outfile, "%c", 1);
	  fprintf(outfile, "%c", 0);
	  fprintf(outfile, "%c", 24);
	  fprintf(outfile, "%c", 0);

	  for (n = 7; n <= 12; n++)
	  {
		  fprintf(outfile, "%c", headers[n] & 0x000000FF);
		  fprintf(outfile, "%c", (headers[n] & 0x0000FF00) >> 8);
		  fprintf(outfile, "%c", (headers[n] & 0x00FF0000) >> 16);
		  fprintf(outfile, "%c", (headers[n] & (unsigned int)0xFF000000) >> 24);
	  }

	  for (y = h - 1; y >= 0; y--)     // BMP image format is written from bottom to top...
	  {
		  for (x = 0; x <= w - 1; x++)
		  {
			  red = pixel_col[y][x].r * 255;
			  green = pixel_col[y][x].g * 255;
			  blue = pixel_col[y][x].b * 255;
			  if (red > 255) red = 255; if (red < 0) red = 0;
			  if (green > 255) green = 255; if (green < 0) green = 0;
			  if (blue > 255) blue = 255; if (blue < 0) blue = 0;

			  fprintf(outfile, "%c", blue);
			  fprintf(outfile, "%c", green);
			  fprintf(outfile, "%c", red);
		  }
		  if (extrabytes)      // BMP lines must be of lengths divisible by 4.
		  {
			  for (n = 1; n <= extrabytes; n++)
			  {
				  fprintf(outfile, "%c", 0);
			  }
		  }
	  }

	  fclose(outfile);
	  return;
  }
  
  void WriteImagePPM(){
    std::ofstream out("out.ppm");
    out<< "P3\n" << w << '\n' << h << '\n' << "255\n";
    for(int y=0;y<h;y++){
      for(int x=0;x<w;x++){
	out << (int)(pixel_col[y][x].r*255) << std::endl;
	out << (int)(pixel_col[y][x].g*255) << std::endl;
	out << (int)(pixel_col[y][x].b*255) << std::endl;
      }
    }
  };
  

};


class Scene{
public:
  void render(){
    sample _sample;
    sampler _sampler(w,h);
    ray _ray; 
    Color color;
    Film film(w,h);
    Camera camera(eyeinit,center);
    vec3 v1(1.,1.,1.),v0(0);
    int progress=-1,counter=0;
    while(_sampler.getSample(_sample)){
      _ray= camera.generateRay(_sample);
      RayTracer::trace(_ray,0,color);
      film.commitColor(_sample,color);
      _sampler.nextSample(_sample);
      counter=_sample.y*100.0/h;
      if(counter!=progress){progress=counter;cout<<progress<<endl;}
    }
    film.writeImageBMP();
    return;
  }
};


int main(int argc,char* argv[]){

  if(argc <2){
    cerr<<"Usage: raytracer scenefile\n";
    exit(-1);
  }
  // For now the scene will be setup in main..to debug
  // This setup should move over to a scene file sitting in the resource folder
  readfile("setup.test");
  STLParser stlParser;
  string src = "resource/characters/Letter_S.stl";
  stlParser.readfile(src.c_str());
  src = "resource/characters/Letter_U.stl";
  stlParser.readfile(src.c_str());
  src = "resource/characters/Letter_S.stl";
  stlParser.readfile(src.c_str());
  src = "resource/characters/Letter_H.stl";
  stlParser.readfile(src.c_str());
  src = "resource/characters/Letter_I.stl";
  stlParser.readfile(src.c_str());
  src = "resource/characters/Letter_L.stl";
  stlParser.readfile(src.c_str());
  double displace = 0;
  cout << objects.size();
  for (int i = 0; i < objects.size(); i++) {
	  for (int j = objects[i].start; j <= objects[i].end; j++) {
		  for (int k = 0; k < 4; k++) {
			  primitives[j].ambient[k] = ambient[k];
			  primitives[j].diffuse[k] = diffuse[k];
			  primitives[j].specular[k] = specular[k];
			  primitives[j].emission[k] = emission[k];
		  }
		  primitives[j].shininess = shininess;
		  primitives[j].transform = cameraTransform*Transform::translate(displace, 0.0, 0.0);
		  primitives[j].invTransform = glm::inverse(primitives[j].transform);
	  }
	  displace += 40;
	  if (i == 4)displace -= 20;
  }
  cout<<"Done reading file, Preprocessing....\n";
  generate_grid();
  cout<<"Done.\n";
  int st = time(NULL);
  Scene scene;
  scene.render();
  int en = time(NULL);
  cout<<"Done\nTime to render: "<<en-st;
  getchar();
}









