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
	long int prev = startingPoint->evaluate();
	unsigned int mem = 0, additional_mem, cost, gain;
	short int *original = (short int*)malloc(problemInstance->nQueries * sizeof(short int));
	short int *indexCounter = (short int *)calloc(problemInstance->nIndexes, sizeof(short int));
	short int query;
	

	std::set<unsigned int> unusedConfigs = std::set<unsigned int>();
	for (unsigned int i = 0; i < problemInstance->nConfigs; ++i)
		unusedConfigs.insert(i);


	// construct indexCounter vector and compute memory occupation of the starting solution
	for (unsigned int i = 0; i < problemInstance->nQueries; i++) {
		original[i] = startingPoint->selectedConfiguration[i];
		unusedConfigs.erase(startingPoint->selectedConfiguration[i]);		// only keep in the set unused configurations
		for (unsigned int j = 0; j < problemInstance->nIndexes; j++) {
			if (original[i] > 0)
				if (problemInstance->configIndexesMatrix[original[i]][j])
				{
					if (indexCounter[j] == 0)
						mem += problemInstance->indexesMemoryOccupation[j];
					indexCounter[j]++;		// counts how many times and index is used by a configuration
				}
		}
	}

	// try each unused configuration
	std::set<unsigned int>::iterator it;
	for (it = unusedConfigs.begin(); it != unusedConfigs.end(); it++)
	{
		cost = 0;
		gain = 0;
		additional_mem = 0;

		for (unsigned int j = 0; j < problemInstance->nIndexes; j++) {		// activate indexes needed by the configuration
			if (problemInstance->configIndexesMatrix[*it][j]) {
				if (indexCounter[j] == 0)
				{
					additional_mem += problemInstance->indexesMemoryOccupation[j];		// considering their memory 
					cost += problemInstance->indexesFixedCost[j];						// and time cost if they're being used for the first time
				}
				indexCounter[j]++;
			}
		}

		if (additional_mem + mem <= problemInstance->M)		// if the solution is still feasible
		{ 
			// activate this configuration for all available queries non-zero gain for that config
			for (unsigned int j = 0; j < problemInstance->queriesWithGain[*it].length; j++) {
				query = problemInstance->queriesWithGain[*it].configs[j];
				if (startingPoint->selectedConfiguration[query] < 0) {		// if the query isn't served by any other config
					startingPoint->selectedConfiguration[query] = *it;
					gain += problemInstance->configQueriesGain[*it][query];
				}
			}

			if (gain > cost) 
			{
				startingPoint->evaluate();		// activated a configuration which provides an improvement, stop
				break;
			}
			else 
			{
				// backtrack and reset the solution vector to it's original status
				for (unsigned int j = 0; j < problemInstance->nQueries; j++)
					startingPoint->selectedConfiguration[j] = original[j];
				// deactivate indexes needed by this config
				for (unsigned int j = 0; j < problemInstance->nIndexes; j++) {
					if (problemInstance->configIndexesMatrix[*it][j])
						indexCounter[j]--;
				}
			}

		}
	}

	unusedConfigs.clear();
	free(original);
	free(indexCounter);

	return startingPoint;
}