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
	unsigned int i, j, k, h, w, mem;
	short int *check = (short int*)calloc(problemInstance->nConfigs,sizeof(short int));
	short int *check2 = (short int*)calloc(problemInstance->nQueries,sizeof(short int));

	mem = 0;
	for (i = 0; i < problemInstance->nConfigs; i++) {		//	iterate x matrix 
		for (j = 0; j < problemInstance->nQueries; j++) {
			

			if (configsServingQueries[i][j]) {	// configuration i serve query j

				//		CHECK FIRST CONSTRAINT
				if (check[i] == 0) {		//	first time for config i 

					for (k = 0; k < problemInstance->nIndexes; k++) {
						if (indexesToBuild[k] == 0 && problemInstance->configIndexesMatrix[i][k] == 1) {
							// index k is served by configuration i so it has to be builded
							// index k is not yet builded
							indexesToBuild[k] = 1;
							// search for memory usage
							mem += problemInstance->indexesMemoryOccupation[k];
						}
					}
					check[i] = 1;
				}

				//		CHECK SECOND CONSTRAINT
				else if(check2[j]==0){		// check culumn query j
					h = 0;
					w = 0;
					while (h < problemInstance->nConfigs) {
						w += configsServingQueries[h][j];
						if (w > 1) return false;
					}
					check2[j] = 1;

				}
				
			}


		}
	}
	
	if (mem > problemInstance->M) return false;
	else return true;

	 
}

unsigned long int Solution::evaluateObjectiveFunction()
{
	unsigned int all_gains = 0;
	unsigned int time_spent = 0;	

	// gets gain given a solution
	for (int i = 0; i < problemInstance->nConfigs; i++) {
		for (int j = 0; j < problemInstance->nQueries; j++) {
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

	// PLEASE NOTE: after finishing the evaluation, remember to save
	// the result inside objFunctionValue (in the solution class) before returning it

}

void Solution::writeToFile(std::string fileName)
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
				fprintf(stdout, "%d ", configsServingQueries[i][j]);	
			}
			fprintf(stdout, "\n");										
			fprintf(fl, "\n");											
		}
		fclose(fl);
	}
}
