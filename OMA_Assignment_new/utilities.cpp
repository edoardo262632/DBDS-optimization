#include "utilities.hpp"


Params parseCommandLine(int argc, char *argv[])
{
	Params execParams = Params();

	if (argc != 2 || argc != 4)
	{
		fprintf(stderr, "Wrong command line parameters usage, expected:"
			"\n$ODBDPsolver_OMAAL_group04.exe <instancefilename> -t <timelimit>\n");
		execParams.parsingError = true;
	}
	else
	{
		for (int i = 1; i < argc; ++i)
		{
			// Parsing the -t <timelimit> parameter, which takes up 2 consecutive args
			if (strcmp(argv[i], "-t") == 0 && i < argc-1)		
			{
				execParams.timeLimit = (unsigned) atoi(argv[i + 1]);
				i++;
			}
			// Parsing the <inputfilename> parameter, also generating the output filename
			else if (execParams.inputFileName.length == 0)		
			{
				execParams.inputFileName = std::string(argv[i]);
				execParams.outputFileName = execParams.inputFileName + "_OMAAL_group04.sol";
			}
			else	// Unknown parameter handling										
			{
				fprintf(stderr, "Unknown usage of command line parameters, expected:"
					"\n$ODBDPsolver_OMAAL_group04.exe <instancefilename> -t <timelimit>\n");
				execParams.parsingError = true;
			}
		}
	}

	return execParams;
}


Instance readInputFile(std::string fileName)
{
	Instance inst;


	return inst;
}


//////////////////////////////////////////////
//
//	Solution class implementation
//
//////////////////////////////////////////////


bool Solution::isFeasible()
{
	// PLEASE NOTE: the third constraint in our model is actually a way to build the 'b' vector
	// which is then used in the objective function evaluation. Therefore, it's not something to check
	// in order to determine the feasibility of the solution but you need to set the proper 0/1 values
	// inside this function
	return false;
}

unsigned long int Solution::evaluateObjectiveFunction()
{
	// PLEASE NOTE: after finishing the evaluation, remember to save
	// the result inside objFunctionValue (in the solution class) before returning it
	return 0;
}

void Solution::writeToFile(std::string fileName)
{

}
