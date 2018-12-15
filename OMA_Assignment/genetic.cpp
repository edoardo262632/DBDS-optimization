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

