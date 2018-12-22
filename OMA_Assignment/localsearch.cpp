#include "localsearch.hpp"


void LocalSearch::setStartingPoint(Solution* sol)
{
	delete startingPoint;
	startingPoint = new Solution(sol);
}

Solution* LocalSearch::run(const Params& parameters)
{
	// First improvement local search implementation
	// using startingPoint as the initial solution for the neighborhood generation
	bool find = false;
	long int prev = startingPoint->evaluate();
	short int *vett = (short int*) malloc(problemInstance->nQueries * sizeof(short int));
	short int original;
	Solution *copia = new Solution(startingPoint);


	//iterate over the queries 
	//for (unsigned int j = 0; j < problemInstance->nQueries; j++) {
	//	original = startingPoint->selectedConfiguration[j]; //save the previous configuration to reset the value if it dosen't improve

	//	// iterate over all serving configuration for that queries
	//	for (unsigned int k = 0; k < problemInstance->configServingQueries[j].length; k++) {
	//		startingPoint->selectedConfiguration[j] = problemInstance->configServingQueries[j].configs[k];
	//		long int val = startingPoint->evaluate();
	//		if (val > prev) {
	//			find = true;
	//			break;
	//		}
	//	}

	//	if (find)
	//		break;
	//	else
	//		startingPoint->selectedConfiguration[j] = original;
	//}


	for (int i = 0; i < problemInstance->nQueries; i++) {
		vett[i] = startingPoint->selectedConfiguration[i];
	}

	for (int i = 0; i < problemInstance->nConfigs; i++) {
		for (int j = 0; j < problemInstance->nQueries; j++)
			if (problemInstance->configQueriesGain[i][j] > 0)
				startingPoint->selectedConfiguration[j] = i;
		if (startingPoint->evaluate() > prev)
			break;
		else
			for (int j = 0; j < problemInstance->nQueries; j++)
				startingPoint->selectedConfiguration[j] = vett[j];
	}

	return startingPoint;
}
