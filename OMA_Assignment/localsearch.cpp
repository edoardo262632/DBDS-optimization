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


	for (unsigned int i = 0; i < problemInstance->nQueries; i++) {
		vett[i] = startingPoint->selectedConfiguration[i];
		for (unsigned int j = 0; j < problemInstance->nIndexes; j++) {
			if (vett[i] > 0)
				if (problemInstance->configIndexesMatrix[vett[i]][j] && b[j] == 0) {
					b[j] = 1;
					mem += problemInstance->indexesMemoryOccupation[j];
				}

		}
	}

	for (unsigned int i = 0; i < problemInstance->nConfigs; i++) {
		prov = 0;
		cost = 0;
		gain = 0;
		for (unsigned int j = 0; j < problemInstance->nIndexes; j++) {
			if (problemInstance->configIndexesMatrix[i][j] && b[j] == 0) {
				b[j] = 1;
				prov += problemInstance->indexesMemoryOccupation[j];
				cost += problemInstance->indexesFixedCost[j];
			}
		}
		if (prov + mem <= problemInstance->M) { // if the solution is still feasible
			// set all avaible queries with gain > 0 to this configuration
			for (unsigned int j = 0; j < problemInstance->queriesWithGain[i].length; j++) {
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
				for (unsigned int j = 0; j < problemInstance->nQueries; j++)
					startingPoint->selectedConfiguration[j] = vett[j];
			}
		}
	}

	free(vett);
	free(b);

	return startingPoint;
}