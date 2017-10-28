#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <deque>
#include <stack>
#include <string>
#include <vector>
#include "Transform.h" 

using namespace std;
#include "variables.h" 
#include "readfile.h"


void rightmultiply(const mat4 & M, stack<mat4> &transfstack) 
{
  mat4 &T = transfstack.top(); 
  T = T * M; 
}
// Function to read the input data values
// Use is optional, but should be very helpful in parsing.  
bool readvals(stringstream &s, const int numvals, double* values) 
{
  for (int i = 0; i < numvals; i++) {
    s >> values[i]; 
    if (s.fail()) {
      cout << "Failed reading value " << i << " will skip, numvals= "<<numvals<<"\n"; 
      return false;
    }
  }
  return true; 
}
void readfile(const char* filename) 
{
  string str, cmd; 
  ifstream in;
  in.open(filename); 
  if (in.is_open()) {

    // This is done using standard STL Templates 
    stack <mat4> transfstack;
    vector <vec3> vertexstack;
    transfstack.push(mat4(1.0));  // identity
    //vertexstack.push_back(vec3(0));  //dummy vertex to sync the indices;
    getline (in, str); 
    while (in) {
      if ((str.find_first_not_of(" \t\r\n") != string::npos) && (str[0] != '#')) {
        // Ruled out comment and blank lines 

        stringstream s(str);
        s >> cmd; 
        int i; 
        double values[10]; // Position and color for light, colors for others
        // Up to 10 params for cameras.  
        bool validinput; // Validity of input 
	// Get the image dimensions
	if (cmd == "size"){
	  validinput=readvals(s,2,values);
	  
	  if(validinput){
	    w=(int)values[0];
	    h=(int)values[1];
	  }
	  else cout<<"error in size reading\n";
	  
	}
	// Get the max recursion depth for raytracing, if provided
	else if (cmd == "maxdepth"){
	  validinput=readvals(s,1,values);

	  if(validinput){
	    maxdepth=values[0];
	  }
	  else cout<<"error in depth reading\n";

	}
	else if(cmd=="attenuation"){
	  validinput=readvals(s,3,values);

	  if(validinput){
	    attenuation[0]=values[0];
	    attenuation[1]=values[1];
	    attenuation[2]=values[2];
	  }
	  else cout<<"error in attenuation reading\n";

	}

	//Get the output image file name, if provided
	else if (cmd == "output"){
	  s>>outfilename;
	}

	//Process the vertices, ignores the maxverts command
	else if(cmd == "vertex"){
	  validinput=readvals(s,3,values);
	  if(validinput){
	    vertexstack.push_back(vec3(values[0],values[1],values[2]));
	  }
	  else cout<<"error in vertex reading\n";

	}
	
        // Process the light, add it to database.
        // Lighting Command
        else if (cmd == "point" || cmd=="directional") {
          if (numused == numLights) { // No more Lights 
            cerr << "Reached Maximum Number of Lights " << numused << " Will ignore further lights\n";
          } else {
            validinput = readvals(s, 6, values); // Position/color for lts.
            if (validinput) {

	      lightposn[numused][0]=values[0];
	      lightposn[numused][1]=values[1];
	      lightposn[numused][2]=values[2];
	      lightposn[numused][3]=cmd=="point"?1:0; //value 1 for point lights
	      lightcolor[numused][0]=values[3];
      	      lightcolor[numused][1]=values[4];
	      lightcolor[numused][2]=values[5];
              ++numused; 
            }
	    else cout<<"error in light reading\n";
          }
        }

        // Material Commands 
        // Ambient, diffuse, specular, shininess properties for each object.
        // Filling this in is pretty straightforward, so I've left it in 
        // the skeleton, also as a hint of how to do the more complex ones.
        // Note that no transforms/stacks are applied to the colors. 

        else if (cmd == "ambient") {
          validinput = readvals(s, 3, values); // colors 
          if (validinput) {
            for (i = 0; i < 3; i++) {
              ambient[i] = values[i]; 
            }
          }
        } else if (cmd == "diffuse") {
          validinput = readvals(s, 3, values); 
          if (validinput) {
            for (i = 0; i < 3; i++) {
              diffuse[i] = values[i]; 
            }
          }
        } else if (cmd == "specular") {
          validinput = readvals(s, 3, values); 
          if (validinput) {
            for (i = 0; i < 3; i++) {
              specular[i] = values[i]; 
            }
          }
        } else if (cmd == "emission") {
          validinput = readvals(s, 3, values); 
          if (validinput) {
            for (i = 0; i < 3; i++) {
              emission[i] = values[i]; 
            }
          }
        } else if (cmd == "shininess") {
          validinput = readvals(s, 1, values); 
          if (validinput) {
            shininess = values[0]; 
          }
        } else if (cmd == "size") {
          validinput = readvals(s,2,values); 
          if (validinput) { 
            w = (int) values[0]; h = (int) values[1]; 
          } 
        } else if (cmd == "camera") {
          validinput = readvals(s,10,values); // 10 values eye cen up fov
          if (validinput) {
 
	    eyeinit[0]=values[0];
	    eyeinit[1]=values[1];
	    eyeinit[2]=values[2];
	    center[0]=values[3];
	    center[1]=values[4];
	    center[2]=values[5];
	    upinit[0]=values[6];
	    upinit[1]=values[7];
	    upinit[2]=values[8];
	    fovy=values[9];
	    fovx=fovy;
	    upinit=Transform::upvector(upinit,center-eyeinit);

          }
	  else cout<<"error in camera reading\n";
        }

        // I've left the code for loading objects in the skeleton, so 
        // you can get a sense of how this works.  
        // Also look at demo.txt to get a sense of why things are done this way.
        else if (cmd == "sphere" || cmd == "tri") {
          if (numobjects == maxobjects) { // No more objects 
            cerr << "Reached Maximum Number of Objects " << numobjects << " Will ignore further objects\n";
          } else {
            validinput = readvals(s, cmd=="tri"?3:4 , values); 
            if (validinput) {
              object * obj = &(objects[numobjects]);

	      if(cmd=="tri"){
		obj->v1=vertexstack[(int)values[0]];
		obj->v2=vertexstack[(int)values[1]];
		obj->v3=vertexstack[(int)values[2]];
		obj->type=TRIANGLE;
	      }
	      else{
		obj->center=vec3(values[0],values[1],values[2]);
		obj->radius=values[3];
		obj->type=SPHERE;
	      }

              // Set the object's light properties
              for (i = 0; i < 4; i++) {
                (obj->ambient)[i] = ambient[i]; 
                (obj->diffuse)[i] = diffuse[i]; 
                (obj->specular)[i] = specular[i]; 
                (obj->emission)[i] = emission[i];
              }
              obj->shininess = shininess; 

              // Set the object's transform
              obj->transform = transfstack.top();
            }
            ++numobjects; 
          }
        }

        else if (cmd == "translate") {
          validinput = readvals(s,3,values); 
          if (validinput) {
	    
            // YOUR CODE FOR HW 2 HERE.  
            // Think about how the transformation stack is affected
            // You might want to use helper functions on top of file. 
            // Also keep in mind what order your matrix is!
	    rightmultiply(Transform::translate(values[0],values[1],values[2]),transfstack);

          }
        }
        else if (cmd == "scale") {
          validinput = readvals(s,3,values); 
          if (validinput) {

            // YOUR CODE FOR HW 2 HERE.  
            // Think about how the transformation stack is affected
            // You might want to use helper functions on top of file.  
            // Also keep in mind what order your matrix is!
	    rightmultiply(Transform::scale(values[0],values[1],values[2]),transfstack);

          }
        }
        else if (cmd == "rotate") {
          validinput = readvals(s,4,values); 
          if (validinput) {

            // YOUR CODE FOR HW 2 HERE. 
            // values[0..2] are the axis, values[3] is the angle.  
            // You may want to normalize the axis (or in Transform::rotate)
            // See how the stack is affected, as above.  
            // Note that rotate returns a mat3. 
            // Also keep in mind what order your matrix is!
	    vec3 axis = vec3(values[0],values[1],values[2]);
	    axis = glm::normalize(axis);
	    rightmultiply(mat4(Transform::rotate(values[3],axis)),transfstack);

          }
        }

        // I include the basic push/pop code for matrix stacks
        else if (cmd == "pushTransform") {
          transfstack.push(transfstack.top()); 
        } else if (cmd == "popTransform") {
          if (transfstack.size() <= 1) {
            cerr << "Stack has no elements.  Cannot Pop\n"; 
          } else {
            transfstack.pop(); 
          }
        }

        else {
          cerr << "Unknown Command: " << cmd << " Skipping \n"; 
        }
      }
      getline (in, str); 
    }

    // Set up initial position for eye, up and amount
    // As well as booleans 

    eye = eyeinit; 
    up = upinit;
    // Store Camera Transformation 
    cameraTransform=Transform::lookAt(eye,center,up);
    // Add ^ to object transformations
    for(int i=0;i<numobjects;i++)
      {
	objects[i].transform=cameraTransform*objects[i].transform;
	objects[i].invTransform=glm::inverse(objects[i].transform);
      }
    // Transform Light positions
    for(int i=0;i<numused;i++){
      lightransf[i]=cameraTransform*lightposn[i];
    }
    globalEye=vec3(cameraTransform*vec4(eye,1));
  } else {
    cerr << "Unable to Open Input Data File " << filename << "\n"; 
    throw 2; 
  }
}
