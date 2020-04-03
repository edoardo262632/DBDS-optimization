#include "localsearch.hpp"

#include <set>


void LocalSearch::setStartingPoint(const Solution& sol)
{
	startingPoint = Solution(sol);
}


Solution LocalSearch::run(const Parameters& parameters)
{
	// First improvement local search implementation
	// using startingPoint as the initial solution for the neighborhood generation
	int mem = startingPoint.memoryCost();
	int extra_mem, cost, gain;

	std::vector<short> original = startingPoint.selectedConfigurations;
	std::set<int> usedConfigs = std::set<int>(original.begin(), original.end());

	std::vector<short> indexCounter = std::vector<short>(problemInstance.nIndexes, 0);

	
	// Count the number of times that each index is used by the 
	// selected configurations serving all the different queries
	for (int i = 0; i < problemInstance.nQueries; i++)
	{
		if (original[i] == -1)
			continue;

		for (int j = 0; j < problemInstance.nIndexes; j++)
		{
			if (problemInstance.configIndexesMatrix[original[i]][j])
				indexCounter[j]++;
		}
	}

	// Iterate over all configurations
	for (int i = 0; i < problemInstance.nConfigs; i++)
	{
		cost = 0, gain = 0;
		extra_mem = 0;

		if (usedConfigs.count(i) == 0)		// If the configuration is not active yet, it could provide a gain for our solution
		{
			for (int j = 0; j < problemInstance.nIndexes; j++)	{	 // Activate indexes needed by the configuration
				if (problemInstance.configIndexesMatrix[i][j]) 
				{
					// If an index is being used for the first time, it has to be built
					if (indexCounter[j] == 0) {							
						extra_mem += problemInstance.indexesMemoryOccupation[j];	// Consider its memory usage
						cost += problemInstance.indexesFixedCost[j];				// ...and its time cost 
					}
					indexCounter[j]++;
				}
			}
		}
		
		if (extra_mem + mem <= problemInstance.M)		// If the solution is still feasible...
		{ 
			// ...activate this configuration for all unserved queries that would benefit from it
			for (int j = 0; j < problemInstance.queriesWithGain[i].size(); j++) {
				int query = problemInstance.queriesWithGain[i][j];
				if (startingPoint.selectedConfigurations[query] < 0) 
				{
					startingPoint.selectedConfigurations[query] = i;
					gain += problemInstance.configQueriesGain[i][query];
				}
			}

			if (gain > cost)	// If the activated configuration has provided an improvement...
			{
				startingPoint.evaluate();		// ...stop the local search
				break;
			}
			else	// Otherwise backtrack and reset the solution to its original status
			{
				startingPoint.selectedConfigurations = original;

				// Deactivate indexes that were built only for this config
				if (usedConfigs.count(i) == 0)
				{
					for (int j = 0; j < problemInstance.nIndexes; j++) {
						if (problemInstance.configIndexesMatrix[i][j])
							indexCounter[j]--;
					}
				}
			}

		}
	}

	return startingPoint;
}