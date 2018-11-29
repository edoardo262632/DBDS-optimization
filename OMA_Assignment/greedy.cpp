#include <time.h>

#include "greedy.hpp"

void Greedy::run(Instance problemInstance, unsigned int time) {
	
	for (unsigned int i = 0; i < problemInstance.nQueries; i++) {

			int A = rand() % problemInstance.nConfigs; // generate a random value for the row to take for each 
			bestSolution.configsServingQueries[i][A] = 1;
			if (!check(problemInstance))
				bestSolution.configsServingQueries[i][A] = 0;
			else
				fprintf(stdout, "Better solution found\n");
	}


}


bool Greedy::check(Instance problemInstance) {
	bool *b = (bool *)calloc(problemInstance.nIndexes, sizeof(bool *));
	unsigned int mem = 0;
	for (unsigned int i = 0; i < problemInstance.nConfigs; i++) {	// slide rows(|C|) of confsServingQueries matrix
		for (unsigned int j = 0; j < problemInstance.nQueries; j++) { // slide collums (|Q|) confsServingQueries matrix
			if (bestSolution.configsServingQueries[i][j]) {
				for (unsigned int k = 0; k < problemInstance.nIndexes; k++) { // slide collum (|I|) of configIndexesMatrix matrix with direct access to the row
					if (problemInstance.configIndexesMatrix[i][k])
						b[k] = true;
				}
			}
		}
	}

	for (unsigned int i = 0; i < problemInstance.nIndexes; i++) {
		if (b[i])
			mem += problemInstance.indexesFixedCost[i];
	}
	if (mem <= problemInstance.M)
		return true;
	else
		return false;
}