#include "greedy.hpp"
#include <time.h>

void Greedy::run(const Instance& problemInstance, unsigned int time) {
	
	for (unsigned int i = 0; i < problemInstance.nQueries; i++) {

		int A = rand() % problemInstance.nConfigs;		// generate a random value for the configuration to take for each query
		bestSolution.configsServingQueries[A][i] = 1;
		if (memoryCost(problemInstance, bestSolution) <= problemInstance.M)
			bestSolution.configsServingQueries[A][i] = 0;							// "backtrack" -> do not activate this configuration
		else
			fprintf(stdout, "Activated configuration (%u) for query (%u)\n", A, i);
	}

	if (bestSolution.isFeasible())
	{
		fprintf(stdout, "\nGreedy solution generation terminated succesfully!\nObjective function value = %lu\nMemory cost = %u\n",
			bestSolution.evaluateObjectiveFunction(), memoryCost(problemInstance, bestSolution));
	}
	else
	{
		fprintf(stderr, "\nGreedy algorithm provided an infeasible solution!\n");
	}
	
}


unsigned int Greedy::memoryCost(const Instance& problemInstance, const Solution& solution) 
{
	bool *b = (bool *)calloc(problemInstance.nIndexes, sizeof(bool *));
	unsigned int mem = 0;

	for (unsigned int i = 0; i < problemInstance.nConfigs; i++) {		// iterate over rows(|C|) of confsServingQueries matrix
		for (unsigned int j = 0; j < problemInstance.nQueries; j++) {	// iterate over columns (|Q|) confsServingQueries matrix
			if (solution.configsServingQueries[i][j])
			{
				for (unsigned int k = 0; k < problemInstance.nIndexes; k++) {	// iterate over column (|I|) of configIndexesMatrix matrix with direct access to the row
					if (problemInstance.configIndexesMatrix[i][k])
						b[k] = true;											// build b[] array for the given solution
				}
			}
		}
	}

	for (unsigned int i = 0; i < problemInstance.nIndexes; i++) {
		if (b[i])
			mem += problemInstance.indexesFixedCost[i];					// calculate memory cost of the given solution
	}

	return mem;
}
