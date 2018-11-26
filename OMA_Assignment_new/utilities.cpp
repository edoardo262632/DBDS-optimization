#include "utilities.hpp"


Params parseCommandLine(int argc, char * argv[])
{
	Params execParams;


	return execParams;
}


Instance readInputFile(std::string fileName)
{	// to TEST
	Instance instance;
	FILE *fl;
	fl = fopen(fileName.c_str(), "r");
	if (fl == NULL) {
			fprintf(stderr,"paolino vuole questo errore");

			return Instance();
	}

	int l = fscanf(fl, "%*s%u",&instance.nQueries);
	l += fscanf(fl, "%*s%u",&instance.nIndexes);
	l += fscanf(fl, "%*s%u",&instance.nConfigs);
	l += fscanf(fl, "%*s%u",&instance.M);
	if (l != 4)
		return Instance();
	
	fscanf(fl, "%*s");
	

	// reading the CONFIGURATION_INDEX_MATRIX
	instance.configIndexesMatrix = (short int**) malloc(instance.nConfigs * sizeof(short int*));
	for (unsigned int i = 0; i < instance.nConfigs; i++) {
		instance.configIndexesMatrix[i] = (short int*) malloc(instance.nIndexes * sizeof(short int));
		for (unsigned int j = 0; j < instance.nIndexes; j++) {
			fscanf(fl, "%hd", &instance.configIndexesMatrix[i][j]);
		}
	}
	fscanf(fl, "%*s");	// eliminate a extra rows

	// alloc and read vector of Fixed_Cost for each index
	instance.indexesFixedCost = (unsigned int*) malloc (instance.nIndexes * sizeof(unsigned int));
	for (unsigned int i = 0; i < instance.nIndexes; i++) {
		fscanf(fl, "%u", &instance.indexesFixedCost[i]);
	}

	fscanf(fl, "%*s"); //  eliminate a extra rows

	// alloc and read vector of Memory needed from every index
	instance.indexesMemoryOccupation = (unsigned int*)malloc(instance.nIndexes * sizeof(unsigned int));
	for (unsigned int i = 0; i < instance.nIndexes; i++) {
		fscanf(fl, "%u", &instance.indexesMemoryOccupation[i]);
	}

	fscanf(fl, "%*s");	//  eliminate a extra rows


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
