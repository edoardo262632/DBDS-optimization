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
		fprintf(stderr, "Encountered an error when attempting to open and read file '%s'\n\n", fileName.c_str());
		system("pause");
		exit(EXIT_FAILURE);
	}

	int l = fscanf(fl, "%*s%u",&instance.nQueries);
	l += fscanf(fl, "%*s%u",&instance.nIndexes);
	l += fscanf(fl, "%*s%u",&instance.nConfigs);
	l += fscanf(fl, "%*s%u",&instance.M);
	if (l != 4) {
		fprintf(stderr, "Error in the instance file format\n");
		return Instance();
	}
	
	fscanf(fl, "%*s");	// eliminate an extra row

	// reading the CONFIGURATION_INDEX_MATRIX
	instance.configIndexesMatrix = (short int**) malloc(instance.nConfigs * sizeof(short int*));
	for (int i = 0; i < instance.nConfigs; i++) {
		instance.configIndexesMatrix[i] = (short int*) malloc(instance.nIndexes * sizeof(short int));
		for (int j = 0; j < instance.nIndexes; j++) {
			fscanf(fl, "%hd", &instance.configIndexesMatrix[i][j]);
		}
	}
	fscanf(fl, "%*s");	// eliminate an extra row

	// alloc and read vector of Fixed_Cost for each index
	instance.indexesFixedCost = (int*) malloc (instance.nIndexes * sizeof(int));
	for (int i = 0; i < instance.nIndexes; i++) {
		fscanf(fl, "%u", &instance.indexesFixedCost[i]);
	}

	fscanf(fl, "%*s"); //  eliminate an extra row

	// alloc and read vector of Memory needed from every index
	instance.indexesMemoryOccupation = (int*)malloc(instance.nIndexes * sizeof(int));
	for (int i = 0; i < instance.nIndexes; i++) {
		fscanf(fl, "%u", &instance.indexesMemoryOccupation[i]);
	}

	fscanf(fl, "%*s");	//  eliminate an extra row

	// alloc and read the CONFIGURATION_QUERIES_GAIN
	instance.configQueriesGain = (int **)malloc(instance.nConfigs * sizeof(int*));
	
	// alloc additional support data structure
	instance.configServingQueries = (CustomArray*)malloc(instance.nQueries * sizeof(CustomArray));
	instance.queriesWithGain = (CustomArray*)malloc(instance.nConfigs * sizeof(CustomArray));

	// set all length of configServingQueries to 0
	for (int j = 0; j < instance.nQueries; j++)
		instance.configServingQueries[j].length = 0;

	// set all length of queriesWithGain to 0
	for (int j = 0; j < instance.nConfigs; j++)
		instance.queriesWithGain[j].length = 0;

	for (int i = 0; i < instance.nConfigs; i++) {
		instance.configQueriesGain[i] = (int*)malloc(instance.nQueries * sizeof(int));
		for (int j = 0; j < instance.nQueries; j++) {
			fscanf(fl, "%u", &instance.configQueriesGain[i][j]);
			// count number of non-zero elements
			if (instance.configQueriesGain[i][j] > 0) {
				instance.configServingQueries[j].length++;
				instance.queriesWithGain[i].length++;
			}
		}
	}

	fclose(fl);

	// population of the configServingQueries data Structure
	for (int i = 0; i < instance.nQueries; i++) {
		int k = 0;
		instance.configServingQueries[i].vector = (int*)calloc(instance.configServingQueries[i].length,sizeof(int));
		for (int j = 0; j < instance.nConfigs; j++) {
			if (instance.configQueriesGain[j][i] > 0)
				instance.configServingQueries[i].vector[k++] = j;
		}
	}

	// population of the configServingQueries data Structure
	for (int i = 0; i < instance.nConfigs; i++) {
		int k = 0;
		instance.queriesWithGain[i].vector = (int*)calloc(instance.queriesWithGain[i].length, sizeof(int));
		for (int j = 0; j < instance.nQueries; j++) {
			if (instance.configQueriesGain[i][j] > 0)
				instance.queriesWithGain[i].vector[k++] = j;
		}
	}

	return instance;
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
	fitnessValue(0),
	problemInstance(probInst)
{	
	selectedConfiguration = (short int*) malloc(problemInstance->nQueries * sizeof(short int));
	for (int i = 0; i < problemInstance->nQueries; i++)
		selectedConfiguration[i] = -1;
	indexesToBuild = (short int*)calloc(problemInstance->nIndexes, sizeof(short int));
}

Solution::Solution(Solution* other)
	: Solution(other->problemInstance)
{
	for (int i = 0; i < problemInstance->nQueries; i++) {
		// copy of the support integer array
		selectedConfiguration[i] = other->selectedConfiguration[i];
	}

	for (int i = 0; i < problemInstance->nIndexes; i++) {
		// copy of the indexesToBuild array
		indexesToBuild[i] = other->indexesToBuild[i];
	}
}

Solution::~Solution()
{
	// free previously allocated heap memory
	free(selectedConfiguration);
	free(indexesToBuild);
}


long int Solution::evaluate() 
{
	int all_gains = 0;
	int time_spent = 0;
	int mem = 0;
	
	for (int i = 0; i < problemInstance->nIndexes; i++)
		indexesToBuild[i] = 0;

	for (int i = 0; i < problemInstance->nQueries; i++) {
		short int x = selectedConfiguration[i];
		if (x >= 0) {
			// iterate on the Indexes vector
			for (int k = 0; k < problemInstance->nIndexes; k++) {
				if (indexesToBuild[k] == 0 && problemInstance->configIndexesMatrix[x][k] == 1) {
					// index k is part of configuration i and has not yet been built, so we need to build it
					indexesToBuild[k] = 1;
				}
			}
			
			all_gains += problemInstance->configQueriesGain[x][i];		// add the contribute of the configuration with the i query
		}
	}

	for (int i = 0; i < problemInstance->nIndexes; i++) 
	{
		if (indexesToBuild[i])
		{
			time_spent += problemInstance->indexesFixedCost[i];
			mem += problemInstance->indexesMemoryOccupation[i];
		}
	}

	bool feasible = mem < problemInstance->M;
	objFunctionValue = feasible ? all_gains - time_spent : LONG_MIN;

	// update fitness value
	fitnessValue = (all_gains - time_spent) -
		(feasible ? 0 : (mem - problemInstance->M));		// penalise infeasible solutions by their surplus memory

	return objFunctionValue;
}


int Solution::memoryCost()
{
	int mem = 0;
	for (int i = 0; i < problemInstance->nIndexes; i++)
		indexesToBuild[i] = 0;

	for (int i = 0; i < problemInstance->nQueries; i++) 
	{
		if (selectedConfiguration[i] >= 0) {
			// iterate on the Indexes vector
			for (int k = 0; k < problemInstance->nIndexes; k++) {
				if (indexesToBuild[k] == 0 && problemInstance->configIndexesMatrix[selectedConfiguration[i]][k] == 1)
					// index k is part of configuration i and has not yet been built, so we need to build it
					indexesToBuild[k] = 1;
			}
		}
	}

	for (int i = 0; i < problemInstance->nIndexes; i++) {
		if (indexesToBuild[i])
			mem += problemInstance->indexesMemoryOccupation[i];				// calculate memory cost of the given solution
	}

	return mem;
}


long int Solution::getObjFunctionValue() const
{
	return objFunctionValue;
}

long int Solution::getFitnessValue() const
{
	return fitnessValue;
}


void Solution::writeToFile(const std::string fileName) const
{
	short int**	configsServingQueries = (short int**)malloc(problemInstance->nQueries * sizeof(short int*));

	// Generate solution matrix on-the-fly before printing it
	for (int i = 0; i < problemInstance->nQueries; i++)
	{
		configsServingQueries[i] = (short int*)calloc(problemInstance->nConfigs, sizeof(short int));

		// set a 1 into the proper cell of the column
		if (selectedConfiguration[i] >= 0)
			configsServingQueries[i][selectedConfiguration[i]] = 1;
	}

	FILE *fl = fopen(fileName.c_str(), "w");
	if (fl == NULL)
		fprintf(stderr, "Error: unable to open file %s", fileName.c_str());		
	else
	{
		for (int i = 0; i < problemInstance->nConfigs; i++) {
			for (int j = 0; j < problemInstance->nQueries; j++) {
				fprintf(fl, "%d ", configsServingQueries[j][i]);		
			}								
			fprintf(fl, "\n");											
		}
		fclose(fl);
	}

	for (int i = 0; i < problemInstance->nQueries; i++)
		free(configsServingQueries[i]);
	free(configsServingQueries);
}
