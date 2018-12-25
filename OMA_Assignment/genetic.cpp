#include "genetic.hpp"


Solution* Genetic::run(const Params& parameters)
{
	unsigned int last_update, last_population_refresh;
	bool ranSearchAfterLastUpdate;
	long long startingTime = getCurrentTime_ms();
	LocalSearch* refiner = new LocalSearch(problemInstance);
	outputFileName = parameters.outputFileName;

	// INITIALIZATION
	start:	fprintf(stdout, "(Re)starting the algorithm...\n"); 
	POPULATION_SIZE = 2 * problemInstance->nQueries;
	initializePopulation();
	
	ranSearchAfterLastUpdate = false;
	generation_counter = 0;
	last_update = 0, last_population_refresh = 0;
	long long currentTime = getCurrentTime_ms();

	// REPEAT UNTIL THERE'S COMPUTATIONAL TIME LEFT (OR ALGORITHM RESTART)
	while (currentTime - startingTime < parameters.timeLimit)
	{
		breedPopulation();
		if (replacePopulationByFitness(parameters.outputFileName, generation_counter))
		{
			ranSearchAfterLastUpdate = false;
			last_update = generation_counter;
		}
		
		// inject new random solutions in the population if it's in a stale state
		//if (generation_counter - last_update > 100 && generation_counter - last_population_refresh > 100)
		//{
		//	last_population_refresh = generation_counter;
		//	replaceLowerHalfPopulation();
		//}

		// run a local search to specialize the population if it's not improving
		if (generation_counter - last_update > 500 && !ranSearchAfterLastUpdate)
		{
			ranSearchAfterLastUpdate = true;
			localSearch(refiner, parameters);
		}
		
		currentTime = getCurrentTime_ms();		// update timestamp and generation number
		generation_counter++;

		// dynamic population size according to generation counter
		if (generation_counter % 5000 == 0 && POPULATION_SIZE > 20)
			POPULATION_SIZE -= POPULATION_SIZE / 5;

		// multistart if stuck in local optima
		if (generation_counter - last_update > MAX_GENERATIONS_BEFORE_RESTART) {
			if (generation_counter > 1.5 * MAX_GENERATIONS_BEFORE_RESTART)
				MAX_GENERATIONS_BEFORE_RESTART = generation_counter;
			goto start;
		}
		
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
	std::vector<int> usedConfigs;					// vector with already used configurations for a parent

	for (unsigned int n = 1; n < POPULATION_SIZE; n++)				// P-1 solutions are initialized with the greedy algorithm
	{
		parents[n] = new Solution(problemInstance);
		usedConfigs.clear();
		// fills the queries in a random order
		for (unsigned int i = 0; i < 2 * problemInstance->nQueries; i++) {
			int j = rand() % problemInstance->nQueries;		// generate a random value for the configuration to take for each query
			parents[n]->selectedConfiguration[j] = maxGainGivenQuery(j);  // gets configuration that results in max gain for a query
			usedConfigs.push_back(parents[n]->selectedConfiguration[j]);  // insert random configuration in the vector

			if (parents[n]->memoryCost() > problemInstance->M) {
				usedConfigs.pop_back();                     // remove the configuration if the memory cost with it is > M
				if (i % 3 == 2) parents[n]->selectedConfiguration[j] = getHighestGainConfiguration(usedConfigs, j);
				if (i % 3 == 1) parents[n]->selectedConfiguration[j] = getRandomConfiguration(usedConfigs, j);
				else parents[n]->selectedConfiguration[j] = -1; // "backtrack" -> do not activate this configuration
			}
		}
		// fills the rest of the queries from "left" to "right"
		for (unsigned int i = 0; i < problemInstance->nQueries; i++) {
			if (parents[n]->selectedConfiguration[i] < 0) {
				parents[n]->selectedConfiguration[i] = maxGainGivenQuery(i);
				usedConfigs.push_back(parents[n]->selectedConfiguration[i]);  // insert random configuration in the vector

				if (parents[n]->memoryCost() > problemInstance->M) {
					usedConfigs.pop_back();                     // remove the configuration if the memory cost with it is > M
					if (i % 3 == 2) parents[n]->selectedConfiguration[i] = getHighestGainConfiguration(usedConfigs, i);
					if (i % 3 == 1) parents[n]->selectedConfiguration[i] = getRandomConfiguration(usedConfigs, i);
					else parents[n]->selectedConfiguration[i] = -1; // "backtrack" -> do not activate this configuration
				}
			}
		}
	}

	// Initialization and evaluation of the starting population set
	for (unsigned int i = 0; i < POPULATION_SIZE; i++) {
		parents[i]->evaluate();
		population.insert(parents[i]);
	}

	checkImprovingSolutions(parents, POPULATION_SIZE);
}



void Genetic::breedPopulation()
{
	// select the best POPULATION_SIZE elements to use as parents
	std::multiset<Solution*, solution_comparator>::iterator it = population.begin();
	for (unsigned int i = 0; it != population.end(); ++it, ++i)
	{
		if (i < POPULATION_SIZE)
			parents[i] = *it;
		else delete *it;
	}

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


bool Genetic::replacePopulationByFitness(const std::string outputFileName, unsigned int gen)
{
	bool updatedBest = false;

	// substitute the old population with the current parents and offsprings
	population.clear();

	for (unsigned int i = 0; i < POPULATION_SIZE; i++)
	{ 
		population.insert(parents[i]);

		offsprings[i]->evaluate();
		population.insert(offsprings[i]);
	}


	return checkImprovingSolutions(offsprings, POPULATION_SIZE);
}


void Genetic::logPopulation()
{
	int n = 1;
	FILE *fl = fopen("populationLog.txt", "w");
	if (fl == NULL)
	{
		fprintf(stderr, "Error: unable to open file log file");
		return;
	}

	// Iterate on the population set and log solutions to file
	std::multiset<Solution*, solution_comparator>::iterator it = population.begin();
	for (unsigned int i = 0; it != population.end() && i < POPULATION_SIZE; ++it, ++i) {
		fprintf(fl, "Generation %u, Solution %d\n", generation_counter, n++);
		fprintf(fl, "		Objective Function value: %ld\n", (*it)->getObjFunctionValue());
		fprintf(fl, "		Memory cost: %u\n", (*it)->memoryCost());
		fprintf(fl, "		Selected configurations: ");
		for (unsigned int i = 0; i < problemInstance->nQueries; i++)
			fprintf(fl, "%d ", (*it)->selectedConfiguration[i]);
		fprintf(fl, "\n");
	}

	fclose(fl);
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
			temp = itemA->selectedConfiguration[i+j];
			itemA->selectedConfiguration[i+j] = itemB->selectedConfiguration[i+j];
			itemB->selectedConfiguration[i+j] = temp;
		}
	}

}


void Genetic::mutate(Solution* sol)
{
	short int randomConfigIndex;
#if !DETERMINISTIC_RANDOM_NUMBER_GENERATION
	srand((unsigned int)getCurrentTime_ms());		// initialize random seed for rand()
#endif

	// Choose probability dynamically based on generation counter
	// Going from an initial split of 10% zero / 90% non-zero to a 90% zero / 10% non-zero 
	// in steps of 5% between generation 1000 to 9000
	int probability = 100 - 5 * (int)(generation_counter / 500);
	if (probability < 10)
		probability = 10;
	else if (probability > 90)
		probability = 90;


	// iterates over the genes
	for (unsigned int i = 0; i < problemInstance->nQueries; i++) {
		// checks if a random generated number (>= 0) is equal to 0. In this case, the mutation occurs
		if (rand() % (problemInstance->nQueries / 2) == 0)
		{
			// chance of choosing another config that servers this query
			if (rand() % 100 < probability) {
				randomConfigIndex = rand() % problemInstance->configServingQueries[i].length;
				sol->selectedConfiguration[i] = problemInstance->configServingQueries[i].configs[randomConfigIndex];
			} 
			// chance of this query being served by "no configuration"
			else {
				sol->selectedConfiguration[i] = -1;
			}
		}
	}
}


void Genetic::localSearch(LocalSearch* refiner, const Params& parameters) 
{
	fprintf(stdout, "Running local search...");
	std::multiset<Solution*, solution_comparator>::iterator it = population.begin();
	std::multiset<Solution*, solution_comparator> newPopulation;
	Solution* tmp;

	for (unsigned int i = 0; it != population.end(); ++it, i++)
	{
		if (i < POPULATION_SIZE)
		{
			refiner->setStartingPoint(*it);
			tmp = new Solution(refiner->run(parameters));
			tmp->evaluate();
			newPopulation.insert(tmp);
		}
		delete *it;
	}

	population.clear();
	population = newPopulation;
	fprintf(stdout, " done\n");
}


void Genetic::replaceLowerHalfPopulation()
{
	std::multiset<Solution*, solution_comparator>::iterator it = population.begin();
	std::multiset<Solution*, solution_comparator> newPopulation;

	// iterates starting in the end of the population
	// and deletes elements of the population in the lower half
	for (unsigned int i = 0; it != population.end(); i++, it++)
	{
		if (i < POPULATION_SIZE / 2)
		{
			newPopulation.insert(*it);
		}
		else delete *it;
	}

	population.clear();
	population = newPopulation;

	// inserts random solution to the lower half of the population
	for (unsigned int i = 0; i < POPULATION_SIZE / 2; i++)
		population.insert(generateRandomSolution());
}


bool Genetic::checkImprovingSolutions(Solution** candidates, int size)
{
	bool updatedBest = false;
	Solution* newBest = bestSolution;

	// check if there's a better solution in the candidates than the current best
	for (int i = 0; i < size; i++)
	{
		if (candidates[i]->getObjFunctionValue() > newBest->getObjFunctionValue())
		{
			updatedBest = true;
			newBest = candidates[i];
		}
	}

	if (updatedBest)
	{
		delete bestSolution;
		bestSolution = new Solution(newBest);		// Update the best solution found so far and log it to file/console
		bestSolution->evaluate();
		bestSolution->writeToFile(outputFileName);
		fprintf(stdout, "Found a new best solution with objective function value = %ld (Generation #%u)\n",
			bestSolution->getObjFunctionValue(), generation_counter);
	}

	return updatedBest;
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
	for (unsigned int i = 0; i < problemInstance->nQueries; i++) 
	{
		int j = rand() % problemInstance->nQueries;		// randomly select a query to modify
		int A = rand() % problemInstance->nConfigs;		// generate a random value for the configuration to take for each query
		sol->selectedConfiguration[j] = A;
		if (sol->memoryCost() > problemInstance->M)
			sol->selectedConfiguration[j] = -1;							// "backtrack" -> do not activate this configuration
	};

	sol->evaluate();
	return sol;
}
