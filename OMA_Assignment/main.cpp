#include <stdlib.h>
#include <stdio.h>
#include <string>

using namespace std;

#include "utilities.hpp"
#include "algorithm.hpp"
#include "genetic.hpp"

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
	
	// Instantiate the proper class and run the algorithm
	Genetic solver = Genetic(&problemInstance);
	Solution* sol = solver.run(executionParameters);
	
	fprintf(stdout, "\nAlgorithm execution terminated succesfully!\nObjective function value = %ld\nMemory cost = %u\n\n",
		sol->getObjFunctionValue(), memoryCost(problemInstance, *sol));

	system("pause");
	return 0;
}