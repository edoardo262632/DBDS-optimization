#include "utilities.hpp"

using namespace std;


Params parseCommandLine(int argc, char *argv[])
{	
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
				execParams.timeLimit = (unsigned) atoi(argv[i + 1])*1000;
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
{	
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
	// alloc additional support data structure
	instance.configServingQueries = (UsefulConfigs*)malloc(instance.nQueries * sizeof(UsefulConfigs));
	for (unsigned int j = 0; j < instance.nQueries; j++)
		instance.configServingQueries[j].length = 0;

	for (unsigned int i = 0; i < instance.nConfigs; i++) {
		instance.configQueriesGain[i] = (unsigned int*)malloc(instance.nQueries * sizeof(unsigned int));
		for (unsigned int j = 0; j < instance.nQueries; j++) {
			fscanf(fl, "%u", &instance.configQueriesGain[i][j]);
			// count number of non-zero elements
			if (instance.configQueriesGain[i][j] > 0) instance.configServingQueries[j].length++;
		}
	}

	fclose(fl);

	// population of the additional support data structure
	for (unsigned int i = 0; i < instance.nQueries; i++) {
		unsigned int k = 0;
		instance.configServingQueries[i].configs = (unsigned int*)calloc(instance.configServingQueries[i].length,sizeof(unsigned int));
		for (unsigned int j = 0; j < instance.nConfigs; j++) {
			if (instance.configQueriesGain[j][i] > 0)
				instance.configServingQueries[i].configs[k++] = j;
		}
	}

	return instance;
}


unsigned int memoryCost(const Instance& problemInstance, const Solution& solution)
{
	bool *b = (bool *)calloc(problemInstance.nIndexes, sizeof(bool *));
	unsigned int mem = 0;

	for (unsigned int i = 0; i < problemInstance.nQueries; i++) {
		short int x = solution.selectedConfiguration[i];
		if (x >= 0) {
			// iterate on the Indexes vector
			for (unsigned int k = 0; k < problemInstance.nIndexes; k++) {
				if (b[k] == 0 && problemInstance.configIndexesMatrix[x][k] == 1) {
					// index k is part of configuration i and has not yet been built, so we need to build it
					b[k] = 1;
				}
			}
		}
	}

	for (unsigned int i = 0; i < problemInstance.nIndexes; i++) {
		if (b[i])
			mem += problemInstance.indexesMemoryOccupation[i];					// calculate memory cost of the given solution
	}

	return mem;
}


long long getCurrentTime_ms()		// returns system time in milliseconds
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
		).count();
}


//////////////////////////////////////////////
//
//	Solution class implementation
//
//////////////////////////////////////////////


Solution::Solution(Instance *probInst)
	: objFunctionValue(LONG_MIN),
	problemInstance(probInst)
{	
	selectedConfiguration = (short int*) malloc(problemInstance->nQueries * sizeof(short int));
	for (unsigned int i = 0; i < problemInstance->nQueries; i++)
		selectedConfiguration[i] = -1;
	indexesToBuild = (short int*)calloc(problemInstance->nIndexes, sizeof(short int));
}

Solution::Solution(const Solution & other)
	: Solution(other.problemInstance)
{
	for (unsigned int i = 0; i < problemInstance->nQueries; i++) {
		// copy of the support integer array
		selectedConfiguration[i] = other.selectedConfiguration[i];
	}

	// copy of the index array
	for (unsigned int i = 0; i < problemInstance->nIndexes; i++)
		indexesToBuild[i] = other.indexesToBuild[i];
}

Solution::~Solution()
{
	// free all previously allocated heap memory
	free(selectedConfiguration);
	free(indexesToBuild);
}


long int Solution::evaluate() {
	
	if (objFunctionValue != LONG_MIN)
		return objFunctionValue;

	unsigned int all_gains = 0;
	unsigned int time_spent = 0;
	unsigned int mem = 0;
	short int *check = (short int*)calloc(problemInstance->nConfigs, sizeof(short int));

	for (unsigned int i = 0; i < problemInstance->nQueries; i++) {
		short int x = selectedConfiguration[i];		// already setted the value to a negative one in the greedy function and swapping fase
		if (x >= 0) {
			if (check[x] == 0) {			// if it's the first time for configuration i 
				check[x] = 1;

				// iterate on the Indexes vector
				for (unsigned int k = 0; k < problemInstance->nIndexes; k++) {
					if (indexesToBuild[k] == 0 && problemInstance->configIndexesMatrix[x][k] == 1) {
						// index k is part of configuration i and has not yet been built, so we need to build it
						indexesToBuild[k] = 1;

						// update memory usage and verify constraint
						mem += problemInstance->indexesMemoryOccupation[k];
						if (mem > problemInstance->M)
							return LONG_MIN;
						time_spent += problemInstance->indexesFixedCost[k];
					}
				}
			}
			all_gains += problemInstance->configQueriesGain[x][i];	// add the contribute of the configuration with the i query
		}
	}

	free(check);

	return all_gains - time_spent;
}

long int Solution::getObjFunctionValue() const
{
	return objFunctionValue;
}


void Solution::writeToFile(const std::string fileName) const
{
	short int**	configsServingQueries = (short int**)malloc(problemInstance->nQueries * sizeof(short int*));

	// Generate solution matrix on-the-fly before printing it
	for (unsigned int i = 0; i < problemInstance->nQueries; i++)
	{
		configsServingQueries[i] = (short int*)calloc(problemInstance->nConfigs, sizeof(short int));

		// set a 1 into the proper cell of the column
		configsServingQueries[i][selectedConfiguration[i]] = 1;
	}

	FILE *fl = fopen(fileName.c_str(), "w");
	if (fl == NULL)
		fprintf(stderr, "Error: unable to open file %s", fileName.c_str());		
	else
	{
		fprintf(stdout, "Found a new best solution with objective function value = %ld", objFunctionValue);
		for (unsigned int i = 0; i < problemInstance->nConfigs; i++) {
			for (unsigned int j = 0; j < problemInstance->nQueries; j++) {
				fprintf(fl, "%d ", configsServingQueries[j][i]);		
			}								
			fprintf(fl, "\n");											
		}
		fclose(fl);
	}

	for (unsigned int i = 0; i < problemInstance->nQueries; i++)
		free(configsServingQueries[i]);
	free(configsServingQueries);
}
