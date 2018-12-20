#include "genetic.hpp"


Solution* Genetic::run(const Params& parameters)
{
	unsigned int generation_counter, last_update;
	long long startingTime = getCurrentTime_ms();

	// INITIALIZATION
	start:	initializePopulation();

	generation_counter = 0;
	last_update = 0;
	long long currentTime = getCurrentTime_ms();

	// REPEAT UNTIL THERE'S COMPUTATIONAL TIME LEFT (OR ALGORITHM RESTART)
	while (currentTime - startingTime < parameters.timeLimit)
	{
		breedPopulation();
		if (evaluateFitness(parameters.outputFileName, generation_counter))
			last_update = generation_counter;
		replacePopulation();
		
		currentTime = getCurrentTime_ms();		// update timestamp

		//fprintf(stdout, "Succesfully trained generation %u - Seconds elapsed: %.4f\n",
		//	generation_counter, ((float)(currentTime-startingTime)/1000));

		generation_counter++;
		if (generation_counter - last_update > MAX_GENERATIONS_BEFORE_RESTART)
			goto start;
	}

	return bestSolution;
}



// Greedy generation and evaluation of the starting population set
void Genetic::initializePopulation()
{
#if !DETERMINISTIC_RANDOM_NUMBER_GENERATION
	srand((unsigned int)getCurrentTime_ms());
#endif

	parents[0] = new Solution(bestSolution);		// one solution is kept with the default configuration
	parents[0]->evaluate();
    std::vector<int> usedConfigs;                  // vector with already used configurations for a parent

	for (int n = 1; n < POPULATION_SIZE; n++)					// P-1 solutions are initialized with the greedy algorithm
	{
		parents[n] = new Solution(problemInstance);          
	    usedConfigs.clear();
		// fills the queries in a random order
		for (unsigned int i = 0; i < 2*problemInstance->nQueries; i++) {
			int j = rand() % problemInstance->nQueries;		// generate a random value for the configuration to take for each query
			parents[n]->selectedConfiguration[j] = maxGainGivenQuery(j);  // gets configuration that results in max gain for a query
			usedConfigs.push_back(parents[n]->selectedConfiguration[j]);  // insert random configuration in the vector

			if (memoryCost(*problemInstance, *parents[n]) > problemInstance->M){
				usedConfigs.pop_back();                     // remove the configuration if the memory cost with it is > M
				if (i % 3 == 2) parents[n]->selectedConfiguration[j] = getHighestGainConfiguration(usedConfigs, j);
				if (i % 3 == 1) parents[n]->selectedConfiguration[j] = getRandomConfiguration(usedConfigs, j);
				else parents[n]->selectedConfiguration[j] = -1; // "backtrack" -> do not activate this configuration
			}
		}
		// fills the rest of the queries from "left" to "right"
		for (unsigned int i = 0; i < problemInstance->nQueries; i++) {
			if (parents[n]->selectedConfiguration[i] < 0){
				parents[n]->selectedConfiguration[i] = maxGainGivenQuery(i);
				usedConfigs.push_back(parents[n]->selectedConfiguration[i]);  // insert random configuration in the vector

				if (memoryCost(*problemInstance, *parents[n]) > problemInstance->M){
					usedConfigs.pop_back();                     // remove the configuration if the memory cost with it is > M
					if (i % 3 == 2) parents[n]->selectedConfiguration[i] = getHighestGainConfiguration(usedConfigs, i);
					if (i % 3 == 1) parents[n]->selectedConfiguration[i] = getRandomConfiguration(usedConfigs, i);
					else parents[n]->selectedConfiguration[i] = -1; // "backtrack" -> do not activate this configuration
				}
			}
		}
	}

	// Initialization and evaluation of the starting population set
	for (int i = 0; i < POPULATION_SIZE; i++) {
		parents[i]->evaluate();
		population.insert(parents[i]);
	}
}



void Genetic::breedPopulation()
{
	// duplicate parents before breeding, so we do not overwrite them
	for (unsigned int i = 0; i < POPULATION_SIZE; i++) {
		offsprings[i] = new Solution(parents[i]);
	}
	
	int N = rand() % 4 + MIN_CROSSOVER_POINTS;			// randomize the number of crossover points to avoid "loops" in subsequent generations
	// apply crossover operator on pairs of parents
	for (unsigned int i = 0; i < POPULATION_SIZE / 2; i++) {
		crossover(offsprings[i], offsprings[POPULATION_SIZE-i-1], N);
	}
	
	// apply mutation operator on all offsprings
	for (unsigned int i = 0; i < POPULATION_SIZE; i++) {
		mutate(offsprings[i]);
	}
}


void Genetic::logPopulation() {
	FILE *file;
	Instance problemInstance;
	unsigned int cost;
	int objFunctionValue;
	short int *selectedConfigurations;
	file = fopen("log.txt", "w");
	if (file == NULL)
		fprintf(stderr, "%s", "Error log file opening");
	return;
	int n = 1;
	for (Solution* sol: population){ //I don't know if it's a good way to iterate a multiset
		//problemInstance = ???
		cost = memoryCost(problemInstance, sol);
		objFunctionValue =sol->objFunctionValue;
		selectedConfigurations = sol->selectedConfiguration;
		fprintf(file, "Solution %d\n", n);
		fprintf(file, "		Memory cost: %d\n", cost);
		fprintf(file, "		Obj Function value: %d\n", objFunctionValue);
		fprintf(file, "		Selected Configuration: ");
		for (int i = 0; i < selectedConfigurations.size(); i++) { //Is the size right??
			fprintf(file, "%d ", selectedConfigurations[i]);
		}
		n++;
	}
}


bool Genetic::evaluateFitness(const std::string outputFileName, unsigned int gen)
{
	long int current_best = bestSolution->evaluate(), val;
	bool found_improving = false;
	int i_best = 0;
	
	for (unsigned int i = 0; i < POPULATION_SIZE; i++)		// offsprings.size()
	{
		val = offsprings[i]->evaluate();
		if (val > current_best)
		{
			current_best = val;			// update current best value
			i_best = i;
			found_improving = true;
		}
	}

	if (found_improving)
	{
		delete bestSolution;
		bestSolution = new Solution(offsprings[i_best]);		// Update the best solution found so far and log it to file/console
		bestSolution->evaluate();
		bestSolution->writeToFile(outputFileName);
		fprintf(stdout, "Found a new best solution with objective function value = %ld (Generation #%u)\n", current_best, gen);
	}
	return found_improving;
}


void Genetic::replacePopulation()
{
	// substitute the old population with the current parents and offsprings
	population.clear();

	for (unsigned int i = 0; i < POPULATION_SIZE; i++)
	{ 
		population.insert(parents[i]);
		population.insert(offsprings[i]);
	}

	// select the best POPULATION_SIZE elements to use as parents
	std::multiset<Solution*, solution_comparator>::iterator it = population.begin();
	for (unsigned int i = 0; it != population.end(); ++it, ++i)
	{
		if (i < POPULATION_SIZE)
			parents[i] = *it;
		else delete *it;
	}
}


void Genetic::crossover(Solution* itemA, Solution* itemB, unsigned int N)
{
	unsigned int M = problemInstance->nQueries / N;
	int temp = 0;

	for (unsigned int i = 0; i < problemInstance->nQueries ; i += 2*M) {
		for (unsigned int j = 0; j < M; j++) {
			if ((j+i) >= problemInstance->nQueries)
				break;

			// swap items between the 2 solutions
			temp = itemA->selectedConfiguration[j];
			itemA->selectedConfiguration[j] = itemB->selectedConfiguration[j];
			itemB->selectedConfiguration[j] = temp;
		}
	}

}


void Genetic::mutate(Solution* sol)
{
	short int randomConfigIndex;
#if !DETERMINISTIC_RANDOM_NUMBER_GENERATION
	srand ((unsigned int)getCurrentTime_ms());		// initialize random seed for rand()
#endif

	// iterates over the genes
	for (unsigned int i = 0; i < problemInstance->nQueries; i++) {
		// checks if a random generated number (>= 0) is equal to 0. In this case, the mutation occurs
		if (rand() % problemInstance->nQueries == 0)
		{
			// 50 percent chance of a config for a query mutating to "no configurations"
			if (rand() % 2 == 0) {
				sol->selectedConfiguration[i] = -1;
			}
			// 50 percent chance of a config for a query mutating to any other config that serves this query
			else {
				randomConfigIndex = rand() % problemInstance->configServingQueries[i].length;
				sol->selectedConfiguration[i] = problemInstance->configServingQueries[i].configs[randomConfigIndex];
			}
		}
	}
}





/* ============================================================================== */
/* ============================================================================== */

// given the configurations already used by a parent, pick one that gives the 
// highest gain to a query
int Genetic::getHighestGainConfiguration(std::vector<int> usedConfigs, int queryIndex)
{
	unsigned int maxGain = 0;
	int maxConfig = -1;
	for (int i = 0; i < usedConfigs.size(); i++){
		if (problemInstance->configQueriesGain[usedConfigs[i]][queryIndex] > maxGain){
			maxGain = problemInstance->configQueriesGain[usedConfigs[i]][queryIndex];
			maxConfig = usedConfigs[i];
		}
	}
	return maxConfig;
}

// given the configurations already used by a parent, pick one randomly (given that its gain is)
// different from 0
int Genetic::getRandomConfiguration(std::vector<int> usedConfigs, int queryIndex)
{
	for (int i = 0; i < usedConfigs.size(); i++){
		int randomConfig = usedConfigs[rand() % usedConfigs.size()];
		if (problemInstance->configQueriesGain[usedConfigs[rand() % usedConfigs.size()]][queryIndex] > 0){
			return randomConfig;
		}
	}
	return -1; // if none, backtrack
}

int Genetic::maxGainGivenQuery(int queryIndex)
{
	unsigned int maxGain = 0;
	int maxConfig = -1;
	for (unsigned int i = 0; i < problemInstance->configServingQueries[queryIndex].length; i++){
		if (problemInstance->configQueriesGain[problemInstance->configServingQueries[queryIndex].configs[i]][queryIndex] > maxGain){
			maxGain = problemInstance->configQueriesGain[problemInstance->configServingQueries[queryIndex].configs[i]][queryIndex];
			maxConfig = problemInstance->configServingQueries[queryIndex].configs[i];
		}
	}
	return maxConfig;
}

Solution * Genetic::generateRandomSolution()
{
	Solution* sol = new Solution(problemInstance);
	for (unsigned int i = 0; i < problemInstance->nQueries; i++) {

		int A = rand() % problemInstance->nConfigs;		// generate a random value for the configuration to take for each query
		sol->selectedConfiguration[i] = A;
		if (memoryCost(*problemInstance, *sol) > problemInstance->M)
			sol->selectedConfiguration[i] = -1;							// "backtrack" -> do not activate this configuration
	};

	return sol;
}