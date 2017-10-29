#include<iostream>

using namespace std;

class STLParser {
public:
	STLParser();
	bool readvals(stringstream &s, const int numvals, double* values);
	void readfile(const char * filename);
};
