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
	
	for (unsigned int i = 0; i < instance.nConfigs; i++) {
		instance.configQueriesGain[i] = (unsigned int*)malloc(instance.nQueries * sizeof(unsigned int));
		for (unsigned int j = 0; j < instance.nQueries; j++)
			fscanf(fl, "%u", &instance.configQueriesGain[i][j]);
	}

	/*
	TODO: creation of additional support data structure
	*/

	fclose(fl);

	return instance;
}


unsigned int memoryCost(const Instance& problemInstance, const Solution& solution)
{
	bool *b = (bool *)calloc(problemInstance.nIndexes, sizeof(bool *));
	unsigned int mem = 0;

	for (int i = 0; i < problemInstance.nQueries; i++) {
		short int x = solution.selectedConfiguration[i];
		if (x >= 0) {
			// iterate on the Indexes vector
			for (int k = 0; k < problemInstance.nIndexes; k++) {
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


//////////////////////////////////////////////
//
//	Solution class implementation
//
//////////////////////////////////////////////


Solution::Solution(Instance *probInst)
	: objFunctionValue(LONG_MIN),
	problemInstance(probInst)
{	
	selectedConfiguration = (short int*) malloc(problemInstance->nQueries*sizeof(short int));
	configsServingQueries = (short int**) malloc(problemInstance->nQueries * sizeof(short int*));
	for (int i = 0; i < problemInstance->nQueries; i++) {
		configsServingQueries[i] = (short int*) calloc(problemInstance->nConfigs, sizeof(short int));
		selectedConfiguration[i] = -1;
	}
	indexesToBuild = (short int*)calloc(problemInstance->nIndexes, sizeof(short int));
}

Solution::Solution(const Solution & other)
	: Solution(other.problemInstance)
{
	for (int i = 0; i < problemInstance->nQueries; i++) {
		// copy of the x matrix
		for (int j = 0; j < problemInstance->nConfigs; j++)
			configsServingQueries[i][j] = other.configsServingQueries[i][j];
		// copy of the support integer array
		selectedConfiguration[i] = other.selectedConfiguration[i];
	}

	// copy of the index array
	for (int i = 0; i < problemInstance->nIndexes; i++)
		indexesToBuild[i] = other.indexesToBuild[i];
}


long int Solution::evaluate() {
	
	if (objFunctionValue != LONG_MIN)
		return objFunctionValue;

	unsigned int all_gains = 0;
	unsigned int time_spent = 0;
	unsigned int mem = 0;
	short int *check = (short int*)calloc(problemInstance->nConfigs, sizeof(short int));

	for (int i = 0; i < problemInstance->nQueries; i++) {
		short int x = selectedConfiguration[i];		// already setted the value to a negative one in the greedy function and swapping fase
		if (x >= 0) {
			if (check[x] == 0) {			// if it's the first time for configuration i 
				check[x] = 1;

				// iterate on the Indexes vector
				for (int k = 0; k < problemInstance->nIndexes; k++) {
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
			all_gains += problemInstance->configQueriesGain[x][i]; //  add the contribute of the configuration with the i query
		}
	}
	return all_gains - time_spent;
}


bool Solution::isFeasible()
{				
	unsigned int i, j, k, mem = 0;
	short int *check = (short int*)calloc(problemInstance->nConfigs,sizeof(short int));
	short int *check2 = (short int*)calloc(problemInstance->nQueries,sizeof(short int));

	for (i = 0; i < problemInstance->nQueries; i++) {		//	iterate over solution matrix 
		for (j = 0; j < problemInstance->nConfigs; j++) {
			
			if (configsServingQueries[i][j]) {		// configuration i serves query j
				
				//	CHECK UNIQUENESS CONSTRAINT
				if (check2[j] > 0)
					return false;
				check2[j]++;

				//	CHECK MEMORY CONSTRAINT
				if (check[i] == 0) {			// if it's the first time for configuration i 
					check[i] = 1;

					// iterate on the Indexes vector
					for (k = 0; k < problemInstance->nIndexes; k++) {  
						if (indexesToBuild[k] == 0 && problemInstance->configIndexesMatrix[i][k] == 1) {
							// index k is part of configuration i and has not yet been built, so we need to build it
							indexesToBuild[k] = 1;
							// update memory usage and verify constraint
							mem += problemInstance->indexesMemoryOccupation[k];
							if (mem > problemInstance->M) 
								return false;
						}
					}

				} 
			}
		}
	}
	return true;	 
}


long int Solution::evaluateObjectiveFunction()
{
	unsigned int all_gains = 0;
	unsigned int time_spent = 0;	

	// gets gain given a solution
	for (int i = 0; i < problemInstance->nConfigs; i++) {
		for (int j = 0; j < problemInstance->nQueries; j++) {
			if(configsServingQueries[i][j])
				all_gains += (configsServingQueries[i][j])*(problemInstance->configQueriesGain[i][j]);
		}
	}

	// gets time spent given a solution
	for (int i = 0; i < problemInstance->nIndexes; i++){
		time_spent += indexesToBuild[i]*(problemInstance->indexesFixedCost[i]);
	}

	// updates objective function value
	objFunctionValue = all_gains - time_spent;

	return objFunctionValue;
}


void Solution::writeToFile(const std::string fileName) const
{
	FILE *fl;
	fl = fopen(fileName.c_str(), "w");
	if (fl == NULL)
		fprintf(stderr, "Error: unable to open file %s", fileName.c_str());		
	else
	{
		fprintf(stdout, "Found a new solution with objective function value = %d", objFunctionValue);
		for (int i = 0; i < problemInstance->nConfigs; i++) {					
			for (int j = 0; j < problemInstance->nQueries; j++) {				
				fprintf(fl, "%d ", configsServingQueries[i][j]);		
			}								
			fprintf(fl, "\n");											
		}
		fclose(fl);
	}
}
