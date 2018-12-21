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
	short int original;

	//iterate over the queries 
	for (int j = 0; j < problemInstance->nQueries; j++) {
		original = startingPoint->selectedConfiguration[j]; //save the previous configuration to reset the value if it dosen't improve

		// iterate over all serving configuration for that queries
		for (int k = 0; k < problemInstance->configServingQueries[j].length; k++) {
			startingPoint->selectedConfiguration[j] = problemInstance->configServingQueries[j].configs[k];
			int val = startingPoint->evaluate();
			if (val > prev) {
				find = true;
				break;
			}
		}

		if (find)
			break;
		else
			startingPoint->selectedConfiguration[j] = original;
	}

	return startingPoint;
}
