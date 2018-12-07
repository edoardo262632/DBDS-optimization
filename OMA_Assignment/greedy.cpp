#include "greedy.hpp"

void Greedy::run(const Instance& problemInstance, const Params& parameters) {
	unsigned int m;
	for (unsigned int i = 0; i < problemInstance.nQueries; i++) {

		int A = rand() % problemInstance.nConfigs;		// generate a random value for the configuration to take for each query
		bestSolution.configsServingQueries[A][i] = 1;
		if ((m = memoryCost(problemInstance, bestSolution)) > problemInstance.M)
			bestSolution.configsServingQueries[A][i] = 0;							// "backtrack" -> do not activate this configuration
		else
			fprintf(stdout, "Activated configuration (%u) for query (%u) %u\n", A, i,m);
	}

	if (bestSolution.isFeasible())
	{
		fprintf(stdout, "\nGreedy solution generation terminated succesfully!\nObjective function value = %ld\nMemory cost = %u\n",
			bestSolution.evaluateObjectiveFunction(), memoryCost(problemInstance, bestSolution));
		bestSolution.writeToFile(parameters.outputFileName);
	}
	else
	{
		fprintf(stderr, "\nGreedy algorithm provided an infeasible solution!\n");
	}
	
}
