#ifdef MAINPROGRAM 
#define EXTERN 
#else 
#define EXTERN extern 
#endif 

#define TRIANGLE 0
#define SPHERE 1

EXTERN int amount; // The amount of rotation for each arrow press
EXTERN vec3 eye; // The (regularly updated) vector coordinates of the eye
EXTERN vec3 globalEye;
EXTERN vec3 up;  // The (regularly updated) vector coordinates of the up

// New mat4 to store camera transform
EXTERN mat4 cameraTransform;

#ifdef MAINPROGRAM 
vec3 eyeinit(0.0,0.0,5.0) ; // Initial eye position, also for resets
vec3 upinit(0.0,1.0,0.0) ; // Initial up position, also for resets
vec3 center(0.0,0.0,0.0) ; // Center look at point 
int w = 600, h = 400 ; // width and height 
float fovy = 90.0,fovx=90.0 ; // For field of view
float rayOffset = 1/500;  //The Camera class sets this value
#else 
EXTERN vec3 eyeinit ; 
EXTERN vec3 upinit ; 
EXTERN vec3 center ;
EXTERN int w, h ; 
EXTERN float fovy,fovx ;
EXTERN float rayOffset ;
#endif 


// Lighting parameter array, similar to that in the fragment shader
const int numLights = 10 ; 
EXTERN vec4 lightposn[numLights] ; // Light Positions
EXTERN vec4 lightcolor[numLights] ; // Light Colors
EXTERN vec4 lightransf[numLights] ; // Lights transformed by modelview
EXTERN int numused ;                     // How many lights are used 

// Materials (read from file) 
// With multiple objects, these are colors for each.
#ifdef MAINPROGRAM
double ambient[4]={0.2,0.2,0.2,0};
vec3 attenuation(1,0,0);
#else
EXTERN double ambient[4];
EXTERN vec3 attenuation;
#endif
EXTERN double diffuse[4] ; 
EXTERN double specular[4] ; 
EXTERN double emission[4] ; 
EXTERN double shininess ; 

// Distinction is now made between primitives and objects.
// All prims in a object will have the same material properties
EXTERN struct object {
	int start, end;
};
EXTERN vector<object> objects;

// For multiple primitives, read from a file.  
const int maxprimitives = 500000 ; 
EXTERN int numprimitives ; 
EXTERN struct primitive {
  int type; //triangle or sphere 
  vec3 v1,v2,v3,center;
  double radius;
  double ambient[4] ; 
  double diffuse[4] ; 
  double specular[4] ;
  double emission[4] ; 
  double shininess ;
  mat4 transform,invTransform ; 
} primitives[maxprimitives] ;

#ifdef MAINPROGRAM
int maxdepth=5;
std::string outfilename="raytrace.png";
#else
EXTERN int maxdepth;
EXTERN std::string outfilename;
#endif

// Variables for grid based accelerating structure
EXTERN vector< vector<int> >grid;
EXTERN double gdx,gdy,gdz;
EXTERN int Nx,Ny,Nz;
EXTERN double offset_x,offset_y,offset_z;
#ifdef MAINPROGRAM
int grid_resolution_parameter=8;
#else
EXTERN int grid_resolution_parameter;
#endif
