#include "genetic.hpp"


void Genetic::run(const Instance & problemInstance, const Params & parameters)
{
	unsigned int generation_counter = 0;
	long long startingTime = getCurrentTime_ms();

	// INITIALIZATION
	initializePopulation(POPULATION_SIZE);

	long long currentTime = getCurrentTime_ms();

	// REPEAT UNTIL THERE'S COMPUTATIONAL TIME LEFT
	while (currentTime - startingTime < parameters.timeLimit)
	{
		breedPopulation(POPULATION_SIZE);
		evaluateFitness(POPULATION_SIZE, parameters.outputFileName);
		replacePopulation(POPULATION_SIZE);
		
		currentTime = getCurrentTime_ms();		// update timestamp

		//fprintf(stdout, "Succesfully trained generation %u - Seconds elapsed: %.4f\n",
		//	generation_counter, ((float)(currentTime-startingTime)/1000));
		generation_counter++;
	}

	fprintf(stdout, "\nGenetic algorithm terminated succesfully!\nObjective function value = %ld\nMemory cost = %u\n\n",
		bestSolution->evaluate(), memoryCost(problemInstance, *bestSolution));
}



void Genetic::initializePopulation(int size)
{
#if !DETERMINISTIC_RANDOM_NUMBER_GENERATION
	srand((unsigned int)time(NULL));
#endif

	parents[0] = new Solution(*bestSolution);		// one solution is kept with the default configuration

	for (int n = 1; n < size; n++)					// P-1 solutions are initialized with the greedy algorithm
	{
		parents[n] = new Solution(&problemInstance);
		for (unsigned int i = 0; i < problemInstance.nQueries; i++) {

			int A = rand() % problemInstance.nConfigs;		// generate a random value for the configuration to take for each query
															// TODO: randomize not over all the configurations but only over the useful ones ??
			parents[n]->selectedConfiguration[i] = A;
			if (memoryCost(problemInstance, *parents[n]) > problemInstance.M)
				parents[n]->selectedConfiguration[i] = -1;						// "backtrack" -> do not activate this configuration
		}
	}

	// Initialization and evaluation of the starting population set
	for (int i = 0; i < size; i++) {
		parents[i]->evaluate();
		population.insert(parents[i]);
	}
}



void Genetic::breedPopulation(int size)
{
	int i;
	std::set<Solution*, solution_comparator>::iterator it;

	// select the best POPULATION_SIZE elements to use as parents
	for (i = 0, it = population.begin(); 
		it != population.end() && i < POPULATION_SIZE; ++it, ++i)
	{
		parents[i] = new Solution(**it);
	}
	
	// duplicate parents before breeding, so we do not overwrite them
	for (int i = 0; i < size; i++) {
		offsprings[i] = new Solution(*parents[i]);
	}
	
	// apply crossover operator on pairs of parents
	for (int i = 0; i < size / 2; i++) {
		crossover(offsprings[i], offsprings[size-i-1], N_CROSSOVER_POINTS);
	}
	
	// apply mutation operator on all offsprings
	for (int i = 0; i < size; i++) {
		mutate(offsprings[i]);
	}
}


void Genetic::evaluateFitness(int size, const std::string outputFileName)
{
	long int current_best = bestSolution->evaluate(), val;
	bool found_improving = false;
	int i_best = 0;
	
	for (int i = 0; i < size; i++)		// offsprings.size()
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
		bestSolution = offsprings[i_best];				// Update the best solution found so far and log it to file/console
		bestSolution->writeToFile(outputFileName);
	}
}


void Genetic::replacePopulation(int size)
{
	population.clear();

	for (int i = 0; i < size; i++) 
	{ 
		population.insert(parents[i]);
		population.insert(offsprings[i]);
	}
}


void Genetic::crossover(Solution* itemA, Solution* itemB, unsigned int N)
{
	unsigned int M = problemInstance.nQueries / N;
	int temp = 0;

	for (unsigned int i = 0; i < problemInstance.nQueries ; i += 2*M) {
		for (unsigned int j = 0; j < M; j++) {
			if ((j+i) >= problemInstance.nQueries)
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
	srand ((unsigned int)time(NULL));		// initialize random seed for rand()
#endif

	// iterates over the genes
	for (unsigned int i = 0; i < problemInstance.nQueries; i++) {
		// checks if a random generated number (>= 0) is equal to 0. In this case, the mutation occurs
		if (rand() % problemInstance.nQueries == 0)
		{
			// 50 percent chance of a config for a query mutating to "no configurations"
			if (rand() % 2 == 0) {
				sol->selectedConfiguration[i] = -1;
			}
			// 50 percent chance of a config for a query mutating to any other config that serves this query
			else {
				randomConfigIndex = rand() % problemInstance.configServingQueries[i].length; 
				sol->selectedConfiguration[i] = problemInstance.configServingQueries[i].configs[randomConfigIndex];
			}
		}
	}
}

// "smarter" version of initialize population, selecting only the usefull
// configurations for a query instead of completely random ones
void Genetic::initializePopulation2(int size)
{
#if !DETERMINISTIC_RANDOM_NUMBER_GENERATION
	srand((unsigned int)time(NULL));
#endif

	parents[0] = new Solution(*bestSolution);		// one solution is kept with the default configuration

	for (int n = 1; n < size; n++)					// P-1 solutions are initialized with the greedy algorithm
	{
		parents[n] = new Solution(&problemInstance);

		for (unsigned int i = 0; i < problemInstance.nQueries; i++) {

			int A = rand() % problemInstance.configServingQueries[i].length; // given the configurations that serves a query, selects one randomly
			parents[n]->selectedConfiguration[i] = problemInstance.configServingQueries[i].configs[A];

			if (memoryCost(problemInstance, *parents[n]) > problemInstance.M)
				parents[n]->selectedConfiguration[i] = -1;
		}
	}

	// Initialization and evaluation of the starting population set
	long int eval_parent;
	for (int i = 0; i < size; i++) {
		eval_parent = parents[i]->evaluate();
		population.insert(parents[i]);
	}
}

// mix between the original initializePopulation(), getHighestGainConfiguration() and getRandomConfiguration()
void Genetic::initializePopulation3(int size)
{
#if !DETERMINISTIC_RANDOM_NUMBER_GENERATION
	srand((unsigned int)time(NULL));
#endif

	parents[0] = new Solution(*bestSolution);		// one solution is kept with the default configuration
	int randomConfigIndex;
    std::vector<int> usedConfigs;                  // vector with already used configurations for a parent

	for (int n = 1; n < size; n++)					// P-1 solutions are initialized with the greedy algorithm
	{
		parents[n] = new Solution(&problemInstance);
	    usedConfigs.clear();

		for (unsigned int i = 0; i < problemInstance.nQueries; i++) {

			int A = rand() % problemInstance.nConfigs;		// generate a random value for the configuration to take for each query
			parents[n]->selectedConfiguration[i] = A;
			usedConfigs.push_back(A);                       // insert random configuration in the vector

			if (memoryCost(problemInstance, *parents[n]) > problemInstance.M){
				usedConfigs.pop_back();                     // remove the configuration if the memory cost with it is > M
				if (i % 3 == 1) parents[n]->selectedConfiguration[i] = getHighestGainConfiguration(usedConfigs, i);
				if (i % 3 == 2) parents[n]->selectedConfiguration[i] = getRandomConfiguration(usedConfigs, i);
				else parents[n]->selectedConfiguration[i] = -1; // "backtrack" -> do not activate this configuration
			}
		}
	}

	// Initialization and evaluation of the starting population set
	long int eval_parent;
	for (int i = 0; i < size; i++) {
		eval_parent = parents[i]->evaluate();
		printf("2 The evaluation of parent is %ld for query %d\n", eval_parent, i);
		for (int j = 0; j < problemInstance.nQueries; j++){
			printf("%d", parents[i]->selectedConfiguration[j]);
		}
		printf("\n");
		population.insert(parents[i]);
	}
}

// given the configurations already used by a parent, pick one that gives the 
// highest gain to a query
int Genetic::getHighestGainConfiguration(std::vector<int> usedConfigs, int queryIndex)
{
	unsigned int maxGain = 0;
	int maxConfig = -1;
	for (int i = 0; i < usedConfigs.size(); i++){
		if (problemInstance.configQueriesGain[usedConfigs[i]][queryIndex] > maxGain){
			maxGain = problemInstance.configQueriesGain[usedConfigs[i]][queryIndex];
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
		if (problemInstance.configQueriesGain[usedConfigs[rand() % usedConfigs.size()]][queryIndex] > 0){
			return randomConfig;
		}
	}
	return -1; // if none, backtrack
}