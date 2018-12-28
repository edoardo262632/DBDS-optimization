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
	int mem = 0, additional_mem, cost, gain;
	std::set<int> alreadyUsedConfigs = std::set<int>();
	short int *original = (short int*)malloc(problemInstance->nQueries * sizeof(short int));
	short int *indexCounter = (short int *)calloc(problemInstance->nIndexes, sizeof(short int));
	

	// construct indexCounter vector and compute memory occupation of the starting solution
	for (int i = 0; i < problemInstance->nQueries; i++) {
		original[i] = startingPoint->selectedConfiguration[i];
		alreadyUsedConfigs.insert(startingPoint->selectedConfiguration[i]);		// save used configurations in the set
		for (int j = 0; j < problemInstance->nIndexes; j++) {
			if (original[i] > 0)
				if (problemInstance->configIndexesMatrix[original[i]][j])
				{
					if (indexCounter[j] == 0)
						mem += problemInstance->indexesMemoryOccupation[j];
					indexCounter[j]++;		// counts how many times and index is used by a configuration
				}
		}
	}

	// iterate over all configurations
	for (int i = 0; i < problemInstance->nConfigs; i++)
	{
		cost = 0, gain = 0;
		additional_mem = 0;

		if (alreadyUsedConfigs.count(i) == 0)		// the configuration hasn't been built yet
		{
			for (int j = 0; j < problemInstance->nIndexes; j++)	{		// activate indexes needed by the configuration
				if (problemInstance->configIndexesMatrix[i][j]) 
				{
					if (indexCounter[j] == 0) {
						additional_mem += problemInstance->indexesMemoryOccupation[j];		// considering their memory 
						cost += problemInstance->indexesFixedCost[j];						// and time cost if they're being used for the first time
					}
					indexCounter[j]++;
				}
			}
		}
		
		if (additional_mem + mem <= problemInstance->M)		// if the solution is still feasible
		{ 
			// activate this configuration for all available queries non-zero gain for that config
			for (int j = 0; j < problemInstance->queriesWithGain[i].length; j++) {
				int query = problemInstance->queriesWithGain[i].vector[j];
				if (startingPoint->selectedConfiguration[query] < 0) {		// if the query isn't served by any other config
					startingPoint->selectedConfiguration[query] = i;
					gain += problemInstance->configQueriesGain[i][query];
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
				for (int j = 0; j < problemInstance->nQueries; j++)
					startingPoint->selectedConfiguration[j] = original[j];

				// deactivate indexes needed by this config, if it had to be built from scratch
				if (alreadyUsedConfigs.count(i) == 0)
				{
					for (int j = 0; j < problemInstance->nIndexes; j++) {
						if (problemInstance->configIndexesMatrix[i][j])
							indexCounter[j]--;
					}
				}
			}

		}
	}

	alreadyUsedConfigs.clear();
	free(original);
	free(indexCounter);

	return startingPoint;
}