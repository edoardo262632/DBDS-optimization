#include <iostream>
#include <exception>

#include "utilities.hpp"
#include "genetic.hpp"


int main(int argc, char **argv)
{
	Parameters executionParameters;
	Instance problemInstance;

	try
	{	// Command line parameters parsing
		executionParameters = parseCommandLine(argc, argv);

		// Read problem instance from input file
		problemInstance.readInputFile(executionParameters.inputFileName);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
	
	// Instantiate the proper class and run the algorithm
	Genetic solver(problemInstance);
	Solution solution = solver.run(executionParameters);
	
	std::cout << "\nAlgorithm execution terminated succesfully!"
		<< "\nObjective function value = " << solution.getObjFunctionValue()
		<< "\nMemory cost = " << solution.memoryCost()
		<< std::endl << std::endl;

	return 0;
}
