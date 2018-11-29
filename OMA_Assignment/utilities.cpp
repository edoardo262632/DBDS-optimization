#include "utilities.hpp"

using namespace std;


Params parseCommandLine(int argc, char *argv[])
{	// working parsing_CommandLine

	Params execParams = Params();

	if (argc != 2 && argc != 4)
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
			else if (execParams.inputFileName.length() == 0)		
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
{	// working reading_file

	Instance instance = Instance();
	FILE *fl;

	fl = fopen(fileName.c_str(), "r");
	if (fl == NULL) 
	{
		fprintf(stderr, "Encountered an error when attempting to open and read file '%s'\n", fileName.c_str());
		return Instance();
	}

	int l = fscanf(fl, "%*s%u",&instance.nQueries);
	l += fscanf(fl, "%*s%u",&instance.nIndexes);
	l += fscanf(fl, "%*s%u",&instance.nConfigs);
	l += fscanf(fl, "%*s%u",&instance.M);
	if (l != 4) {
		fprintf(stderr, "Error in the file format\n");
		return Instance();
	}
	
	fscanf(fl, "%*s");	// eliminate an extra row

	// reading the CONFIGURATION_INDEX_MATRIX
	instance.configIndexesMatrix = (short int**) malloc(instance.nConfigs * sizeof(short int*));
	for (unsigned int i = 0; i < instance.nConfigs; i++) {
		instance.configIndexesMatrix[i] = (short int*) malloc(instance.nIndexes * sizeof(short int));
		for (unsigned int j = 0; j < instance.nIndexes; j++) {
			fscanf(fl, "%hd", &instance.configIndexesMatrix[i][j]);
		}
	}
	fscanf(fl, "%*s");	// eliminate an extra row

	// alloc and read vector of Fixed_Cost for each index
	instance.indexesFixedCost = (unsigned int*) malloc (instance.nIndexes * sizeof(unsigned int));
	for (unsigned int i = 0; i < instance.nIndexes; i++) {
		fscanf(fl, "%u", &instance.indexesFixedCost[i]);
	}

	fscanf(fl, "%*s"); //  eliminate an extra row

	// alloc and read vector of Memory needed from every index
	instance.indexesMemoryOccupation = (unsigned int*)malloc(instance.nIndexes * sizeof(unsigned int));
	for (unsigned int i = 0; i < instance.nIndexes; i++) {
		fscanf(fl, "%u", &instance.indexesMemoryOccupation[i]);
	}

	fscanf(fl, "%*s");	//  eliminate an extra row

	// alloc and read the CONFIGURATION_QUERIES_GAIN
	instance.configQueriesGain = (unsigned int **)malloc(instance.nConfigs * sizeof(unsigned int*));
	for (unsigned int i = 0; i < instance.nConfigs; i++) {
		instance.configQueriesGain[i] = (unsigned int*)malloc(instance.nQueries * sizeof(unsigned int));
		for (unsigned int j = 0; j < instance.nQueries; j++)
			fscanf(fl, "%u", &instance.configQueriesGain[i][j]);
	}

	fclose(fl);

	return instance;
}


//////////////////////////////////////////////
//
//	Solution class implementation
//
//////////////////////////////////////////////


Solution::Solution(Instance *probInst)
	: objFunctionValue(0),
	problemInstance(probInst)
{
	configsServingQueries = (short int**)malloc(problemInstance->nConfigs * sizeof(short int*));
	for (int i = 0; i < problemInstance->nConfigs; i++)
		configsServingQueries[i] = (short int*)calloc(problemInstance->nQueries, sizeof(short int));
	indexesToBuild = (short int*)calloc(problemInstance->nIndexes, sizeof(short int));
}


bool Solution::isFeasible()
{
	bool FF;	// posso risparmiarmela

	// Instace = problemInstance

	//	 allocate vector for check 2nd constraint
	unsigned int *check4Query =(unsigned int*) calloc(problemInstance->nQueries,sizeof(unsigned int)); 
					
	unsigned int i, j, k, mem;
	
	// allocate b vector
	// b vector is for all x matrix not for each row
	//this->indexesToBuild = (short int*)calloc(problemInstance->nIndexes,sizeof(short int));
	
	//	iterate x matrix 

	for (i = 0; i < problemInstance->nConfigs; i++) {
		for (j = 0; j < problemInstance->nQueries; j++) {

			if (this->configsServingQueries[i][j] == 1) {			// configuration i serve query j

						///		CHECK 2ND CONSTRAINT
				// fare un check scorrendo le colonne
				if (check4Query[j] = 1) {
					//fprintf(stderr, "Too many configuration for query %u", j);
					return FF = false;
				}					
				else check4Query[j] = 1;
						///		MEMORY CHECK
				for (k = 0; k < problemInstance->nIndexes; k++) {
					if (problemInstance->configIndexesMatrix[i][k] == 1) {
						// index k is served by configuration i so it has to be builded
						indexesToBuild[k] = 1;
						// search for memory usage
						mem += problemInstance->indexesMemoryOccupation[k]; // mettere furoi dal ciclo
						if (mem > problemInstance->M) {
							//fprintf(stderr, "Configuration %u that uses index %u for quey %u exceeds memory limit \n", i, k, j);
							return FF = false;
						}

					}
				}

			}

		}
	}
	
	 



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
	// TODO: please use fprintf() to write the outputs, using stdout for standard output, stderr for errors
	// and the declared file pointer to write on the output file
	// Also, add a console log message that says "Found a new solution with objective function value = X"

	ofstream myfile;
	myfile.open(fileName.c_str());
	if (myfile.is_open())
	{
		for (int i = 0; i < problemInstance->nConfigs; i++) {
			for (int j = 0; j < problemInstance->nQueries; j++) {

				myfile << configsServingQueries[i][j] << " ";
				cout << configsServingQueries[i][j] << " ";
			}
			myfile << "\n";
			cout << "\n";
		}
		myfile.close();
	}
	else cout << "Error: unable to open file";
}
