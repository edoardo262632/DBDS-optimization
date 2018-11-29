#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>

using namespace std;

#include "utilities.hpp"
#include "algorithm.hpp"
#include "greedy.hpp"


int main(int argc, char **argv) 
{
	Params executionParameters;
	Instance problemInstance;

	executionParameters = parseCommandLine(argc, argv);
	if (!executionParameters.parsingError)
	{
		problemInstance = readInputFile(executionParameters.inputFileName);
	}
	else 
	{
		fprintf(stderr, "\nEncountered error when parsing command line parameters, aborting...\n");
		exit (EXIT_FAILURE);
	} 
	
	// TODO: Instantiate the proper class and run the algorithm

	return 0;
}