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
	srand((unsigned int)getCurrentTime_ms());						// initialize seed for rand()
	initializePopulation();

	/* =========================================== */
	// Temporary, needed to see what's happening after
	// algorithm restart, has to be removed !!
	delete bestSolution;
	bestSolution = new Solution(problemInstance);
	bestSolution->evaluate();
	/* =========================================== */

	ranSearchAfterLastUpdate = false;
	generation_counter = 0, last_update = 0;
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

		// periodically run a local search to specialize the population
		if (generation_counter - last_update > 50 && !ranSearchAfterLastUpdate)
		{
			ranSearchAfterLastUpdate = true;
			localSearch(refiner, parameters);
		}
		
		
		// dynamically reducing population size according to generation counter
		//if ((generation_counter+1) % 100 == 0 && POPULATION_SIZE > 20)
		//	POPULATION_SIZE -= POPULATION_SIZE / 10;

		// multistart if stuck in local optima
		if (generation_counter - last_update > MAX_GENERATIONS_BEFORE_RESTART) 
		{
			POPULATION_SIZE = POPULATION_SIZE_MULTIPLIER * problemInstance->nQueries;
			MAX_GENERATIONS_BEFORE_RESTART = generation_counter;

			// empty the population set, remove previous solutions
			for (std::multiset<Solution*, solution_comparator>::iterator it = population.begin();
				it != population.end(); it++)
			{
				delete *it;
				it = population.erase(it);
			}
			population.clear();

			/*free(offsprings);
			free(parents);
			parents = (Solution**)malloc(POPULATION_SIZE * sizeof(Solution*));
			offsprings = (Solution**)malloc(POPULATION_SIZE * sizeof(Solution*));*/

			goto start;
		}

		currentTime = getCurrentTime_ms();		// update timestamp and generation number
		generation_counter++;
	}

	return bestSolution;
}



// Greedy generation and evaluation of the starting population set
void Genetic::initializePopulation()
{
	parents[0] = new Solution(problemInstance);		// one solution is kept with the default configuration
	parents[0]->evaluate();
	std::vector<int> usedConfigs;					// vector with already used configurations for a parent

	for (int n = 1; n < POPULATION_SIZE; n++)				// P-1 solutions are initialized with the greedy algorithm
	{
		parents[n] = new Solution(problemInstance);
		usedConfigs.clear();
		// fills the queries in a random order
		for (int i = 0; i < 2 * problemInstance->nQueries; i++) {
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
		for (int i = 0; i < problemInstance->nQueries; i++) {
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
	for (int i = 0; i < POPULATION_SIZE; i++) {
		parents[i]->evaluate();
		population.insert(parents[i]);
	}

	checkImprovingSolutions(parents, POPULATION_SIZE);
}



void Genetic::breedPopulation()
{
	 // select the best POPULATION_SIZE elements to use as parents
	std::multiset<Solution*, solution_comparator>::iterator it = population.begin();
	for (int i = 0; it != population.end(); ++it, ++i)
	{
		if (i < POPULATION_SIZE)
			parents[i] = *it;
		else delete *it;
	}

	// duplicate parents before breeding, so we do not overwrite them
	for (int i = 0; i < POPULATION_SIZE; i++) {
		offsprings[i] = new Solution(parents[i]);
	}
	
	int N = rand() % 4 + MIN_CROSSOVER_POINTS;			// randomize the number of crossover points to avoid "loops" in subsequent generations
	// apply crossover operator on pairs of parents
	for (int i = 0; i < POPULATION_SIZE / 2; i++) {
		int A = rand() % POPULATION_SIZE;
		int B = rand() % POPULATION_SIZE;
		crossover(offsprings[A], offsprings[B], N);
		//crossover(offsprings[i], offsprings[POPULATION_SIZE - i - 1], N);
	}

	// apply mutation operator on all offsprings
	for (int i = 0; i < POPULATION_SIZE ; i++) {
		mutate(offsprings[i]);
	}
}


bool Genetic::replacePopulationByFitness(const std::string outputFileName, unsigned int gen)
{
	// substitute the old population with the current parents and offsprings
	population.clear();

	for (int i = 0; i < POPULATION_SIZE; i++)
	{ 
		population.insert(parents[i]);

		offsprings[i]->evaluate();
		population.insert(offsprings[i]);
	}

	return checkImprovingSolutions(offsprings, POPULATION_SIZE);
}


void Genetic::crossover(Solution* itemA, Solution* itemB, int N)
{
	int M = problemInstance->nQueries / N;
	int temp = 0;

	for (int i = 0; i < problemInstance->nQueries ; i += 2*M) {
		for (int j = 0; j < M; j++) {
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

	// Choose probability dynamically based on generation counter
	// Going from an initial split of 10% zero / 90% non-zero to a 90% zero / 10% non-zero 
	// in steps of 5% between generation 1000 to 9000
	int probability = 100 - 5 * (int)(generation_counter / 500);
	if (probability < 10)
		probability = 10;
	else if (probability > 90)
		probability = 90;


	// iterates over the genes
	for (int i = 0; i < problemInstance->nQueries; i++) {
		// checks if a random generated number (>= 0) is equal to 0. In this case, the mutation occurs
		if (rand() % problemInstance->nQueries  == 0)
		{
			// chance of choosing another config that servers this query
			if (rand() % 100 < probability) {
				randomConfigIndex = rand() % problemInstance->configServingQueries[i].length;
				sol->selectedConfiguration[i] = problemInstance->configServingQueries[i].vector[randomConfigIndex];
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

	for (int i = 0; it != population.end(); ++it, i++)
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
	int maxGain = 0;
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
	int maxGain = 0;
	int maxConfig = -1;
	for (int i = 0; i < problemInstance->configServingQueries[queryIndex].length; i++){
		if (problemInstance->configQueriesGain[problemInstance->configServingQueries[queryIndex].vector[i]][queryIndex] > maxGain){
			maxGain = problemInstance->configQueriesGain[problemInstance->configServingQueries[queryIndex].vector[i]][queryIndex];
			maxConfig = problemInstance->configServingQueries[queryIndex].vector[i];
		}
	}
	return maxConfig;
}

Solution * Genetic::generateRandomSolution()
{
	Solution* sol = new Solution(problemInstance);
	for (int i = 0; i < problemInstance->nQueries; i++) 
	{
		int j = rand() % problemInstance->nQueries;		// randomly select a query to modify
		int A = rand() % problemInstance->nConfigs;		// generate a random value for the configuration to take for each query
		sol->selectedConfiguration[j] = A;
		if (sol->memoryCost() > problemInstance->M)
			sol->selectedConfiguration[j] = -1;							// "backtrack" -> do not activate this configuration
		else
			for (int x = 0; x < problemInstance->queriesWithGain[A].length; x++)
				if (sol->selectedConfiguration[problemInstance->queriesWithGain[A].vector[x]] == -1)
					sol->selectedConfiguration[problemInstance->queriesWithGain[A].vector[x]] = A;
	};

	sol->evaluate();
	return sol;
}


void Genetic::initializePopulation2()
{
	parents[0] = new Solution(problemInstance);		// one solution is kept with the default configuration
	parents[0]->evaluate();
	std::vector<int> usedConfigs;					// vector with already used configurations for a parent
	int *b = (int *)calloc(problemInstance->nIndexes, sizeof(int));
	int mem, prev;

	for (int k = 1; k < POPULATION_SIZE; k++) {
		mem = 0;
		parents[k] = new Solution(problemInstance);
		usedConfigs.clear();
		for (int i = 0; i < problemInstance->nIndexes; i++)
			b[i] = 0;
		for (int i = 0; i < problemInstance->nQueries; i++) 
		{
			if (parents[k]->selectedConfiguration[i] == -1) {
				int n = rand() % problemInstance->configServingQueries[i].length;
				int conf = problemInstance->configServingQueries[i].vector[n];
				prev = mem;
				for (int j = 0; j < problemInstance->nIndexes; j++) {
					if (problemInstance->configIndexesMatrix[conf][j] && b[j] == 0) {
						mem += problemInstance->indexesMemoryOccupation[j];
					}
				}
				if (mem < problemInstance->M) {
					for (int j = 0; j < problemInstance->nIndexes; j++) {
						if (problemInstance->configIndexesMatrix[conf][j] && b[j] == 0) {
							b[j] = 1;
						}
					}
					for (int x = 0; x < problemInstance->queriesWithGain[conf].length; x++)
						if (parents[k]->selectedConfiguration[problemInstance->queriesWithGain[conf].vector[x]] == -1)
							parents[k]->selectedConfiguration[problemInstance->queriesWithGain[conf].vector[x]] = conf;
				}
				else {	//  backtrack
					mem = prev;
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
