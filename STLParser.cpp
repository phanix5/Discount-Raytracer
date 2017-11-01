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
#include "STLParser.h"



STLParser::STLParser() {
}
// Function to read the input data values
// Use is optional, but should be very helpful in parsing.  
bool STLParser::readvals(stringstream &s, const int numvals, double* values)
{
	for (int i = 0; i < numvals; i++) {
		s >> values[i];
		if (s.fail()) {
			cout << "Failed reading value " << i << " will skip, numvals= " << numvals << "\n";
			return false;
		}
	}
	return true;
}

void STLParser::readfile(const char * filename) {
	string str, cmd;
	ifstream in;
	in.open(filename);
	if (in.is_open()) {

		// This is done using standard STL Templates 
		stack <mat4> transfstack;
		vector <vec3> vertexstack;
		transfstack.push(mat4(1.0));  // identity

		// Create a new object
		object obj;
		//set start index
		obj.start = numprimitives;

		getline(in, str);
		while (in) {
			if ((str.find_first_not_of(" \t") != string::npos) && (str[0] != '#')) {
				// Ruled out comment and blank lines 
				
				stringstream s(str);
				s >> cmd;
				int i;
				double values[10]; // Position and color for light, colors for others
								   // Up to 10 params for cameras.  
				bool validinput; // Validity of input 

				//Process the vertices, ignores the maxverts command
				if (cmd == "vertex") {
					validinput = readvals(s, 3, values);
					if (validinput) {
						vertexstack.push_back(vec3(values[0], values[1], values[2]));
					}
					else cout << "error in vertex reading\n";

				}

				else if (cmd == "endfacet") {
					if (numprimitives == maxprimitives) { // No more primitives 
						cerr << "Reached Maximum Number of primitives " << numprimitives << " Will ignore further primitives\n";
					}
					else {
						primitive * prim = &(primitives[numprimitives]);
						//cout << "yup";

						prim->v1 = vertexstack[0];
						prim->v2 = vertexstack[1];
						prim->v3 = vertexstack[2];
						prim->type = TRIANGLE;
						vertexstack.clear();

						for (i = 0; i < 4; i++) {
							(prim->ambient)[i] = ambient[i];
							(prim->diffuse)[i] = diffuse[i];
							(prim->specular)[i] = specular[i];
							prim->shininess = shininess;
						}
						// Set the primitive's transform
						prim->transform = transfstack.top();
					}
					++numprimitives;
				}
				else {
					//cerr << "Unknown Command: " << cmd << " Skipping \n";
				}
			}

			getline(in, str);
		}

		//set end index in object and push it into the vertex
		obj.end = numprimitives - 1;
		objects.push_back(obj);
	}
	else {
		cerr << "Unable to Open Input Data File " << filename << "\n";
		throw 2;
	}
}

