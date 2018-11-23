#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>

using namespace std;

#include "utilities.hpp"
#include "algorithm.hpp"


int main(int argc, char **argv) 
{
	Params executionParameters;
	Instance problemInstance;

	executionParameters = parseCommandLine(argc, argv);
	problemInstance = readInputFile(executionParameters.inputFileName);

	// TODO: Instantiate the proper class and run the algorithm

	return 0;
}