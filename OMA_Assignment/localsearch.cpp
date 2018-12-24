/*#include "localsearch.hpp"


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
}*/



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
	bool find = false, available = false;
	long int prev = startingPoint->evaluate(), mem = 0, prov, cost, gain;
	short int *vett = (short int*)malloc(problemInstance->nQueries * sizeof(short int));
	short int *b = (short int *)calloc(problemInstance->nIndexes, sizeof(short int));
	short int query;



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
		for (int j = 0; j < problemInstance->nIndexes; j++) {
			if (vett[i] > 0)
				if (problemInstance->configIndexesMatrix[vett[i]][j] && b[j] == 0) {
					b[j] = 1;
					mem += problemInstance->indexesMemoryOccupation[j];
				}

		}
	}

	for (int i = 0; i < problemInstance->nConfigs; i++) {
		prov = 0;
		cost = 0;
		gain = 0;
		for (int j = 0; j < problemInstance->nIndexes; j++) {
			if (problemInstance->configIndexesMatrix[i][j] && b[j] == 0) {
				b[j] = 1;
				prov += problemInstance->indexesMemoryOccupation[j];
				cost += problemInstance->indexesFixedCost[j];
			}
		}
		if (prov + mem <= problemInstance->M) { // if the solution is still feasible
			// set all avaible queries with gain > 0 to this configuration
			for (int j = 0; j < problemInstance->queriesWithGain[i].length; j++) {
				query = problemInstance->queriesWithGain[i].configs[j];
				if (startingPoint->selectedConfiguration[query] < 0) {
					startingPoint->selectedConfiguration[query] = i;
					gain += problemInstance->configQueriesGain[i][query];
				}
			}
			if (gain > cost) {
				startingPoint->evaluate();
				break;
			}
			else {
				for (int j = 0; j < problemInstance->nQueries; j++)
					startingPoint->selectedConfiguration[j] = vett[j];
			}
		}
	}

	free(vett);
	free(b);

	return startingPoint;
}