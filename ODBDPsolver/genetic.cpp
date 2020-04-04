#include "genetic.hpp"

#include <iostream>


Genetic::Genetic(Instance& inst)
	: Algorithm(inst), 
	parameters(nullptr)
{
	// Instantiate algorithm thread classes
	for (int i = 1; i <= N_THREADS; i++)
		threads.emplace_back(GeneticThread(*this, i));
}

Genetic::~Genetic()
{
}


Solution Genetic::run(const Parameters& parameters)
{
	this->parameters = &parameters;

	// Creating multiple threads to run the algorithm in parallel
	std::thread workerThreads[N_THREADS];
	for (int i = 0; i < N_THREADS; i++)
		workerThreads[i] = std::thread(&Genetic::GeneticThread::run, threads[i]);

	// Wait for each thread to terminate its job
	for (int i = 0; i < N_THREADS; i++)
		workerThreads[i].join();

	return bestSolution;
}


void Genetic::updateBestSolution(const Solution& newBest)
{
	mtx.lock();		// LOCK

	if (newBest.getObjFunctionValue() > bestSolution.getObjFunctionValue())
	{
		bestSolution = newBest;

		std::cout << "Found a new best solution with objective function value = "
			<< bestSolution.getObjFunctionValue() << std::endl;

		try 
		{	// Write the new best solution on the output file
			bestSolution.writeToFile(parameters->outputFileName);
		}
		catch (exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	mtx.unlock();	// UNLOCK
}



Genetic::GeneticThread::GeneticThread(Genetic& caller, int tID)
	: algorithm(caller), threadID(tID),
	localBestSolution(Solution(algorithm.problemInstance)),
	population(std::multiset<Solution*, solution_comparator>()),
	parents(), offsprings(), generation_counter(0)
{
}

Genetic::GeneticThread::~GeneticThread()
{
}


void Genetic::GeneticThread::run()
{
	unsigned int last_update;
	long long startingTime = getCurrentTime_ms();
	LocalSearch refiner(algorithm.problemInstance);

	// INITIALIZATION
	start:	fprintf_s(stdout, "Thread %d is (re)starting the algorithm...\n", threadID);

	random_number.seed(std::random_device{}());
	localBestSolution = Solution(algorithm.problemInstance);

	generation_counter = 0, last_update = 0;
	long long currentTime = getCurrentTime_ms();

	// Randomly choosing one of the 2 avaiable initializers
	initializePopulation(random_number() % 2);

	// REPEAT UNTIL THERE'S COMPUTATIONAL TIME LEFT (OR ALGORITHM RESTART)
	while (currentTime - startingTime < algorithm.parameters->timeLimit)
	{
		// Generate offsprings
		breedPopulation();

		// Replace the current population with the best offsprings (and parents)
		if (replacePopulationByFitness())
		{
			last_update = generation_counter;
		}

		// Periodically run a local search to specialize the population
		if (generation_counter - last_update == 50)
		{
			localSearch(refiner);
		}

		// Multi-start technique in case the algorithm gets stuck in a local optimum
		if (generation_counter - last_update > MAX_GENERATIONS_BEFORE_RESTART)
		{
			if (last_update > MAX_GENERATIONS_BEFORE_RESTART)
				MAX_GENERATIONS_BEFORE_RESTART = last_update;

			// Empty the population set, delete all existing solution objects
			for (auto it = population.begin(); it != population.end(); it++)
				delete* it;
			population.clear();

			goto start;
		}

		// Check if the current generation has produced a solution better than the previous best
		if (localBestSolution.getObjFunctionValue() > algorithm.bestSolution.getObjFunctionValue())
		{
			algorithm.updateBestSolution(localBestSolution);
		}

		currentTime = getCurrentTime_ms();		// Update timestamp and generation number
		generation_counter++;
	}
}


// Greedy generations and evaluation of the starting population set
void Genetic::GeneticThread::initializePopulation(int type)
{
	// The first solution is always kept with the default configuration
	parents[0] = new Solution(algorithm.problemInstance);

	switch (type)	// Multiple initializers are available
	{
	case 0:
	{
		// Vector with the set of configurations already used by a solution
		std::vector<int> usedConfigs;			
		usedConfigs.reserve(algorithm.problemInstance.nConfigs);

		for (int n = 1; n < POPULATION_SIZE; n++)			// P-1 solutions are initialized with the greedy algorithm
		{
			usedConfigs.clear();
			parents[n] = new Solution(algorithm.problemInstance);
			
			// Fill the queries in a random order
			for (int i = 0; i < 2 * algorithm.problemInstance.nQueries; i++) 
			{
				int query = random_number() % algorithm.problemInstance.nQueries;			// Select a random query
				parents[n]->selectedConfigurations[query] = maxGainGivenQuery(query);		// Get the best configuration for that query
				usedConfigs.emplace_back(parents[n]->selectedConfigurations[query]);		// Keep track of the configurations used

				if (parents[n]->memoryCost() > algorithm.problemInstance.M)		// Remove the configuration if it raises the memory cost > M
				{
					usedConfigs.pop_back();										
					if (i % 3 == 2) parents[n]->selectedConfigurations[query] = getHighestGainConfiguration(usedConfigs, query);
					if (i % 3 == 1) parents[n]->selectedConfigurations[query] = getRandomConfiguration(usedConfigs, query);
					else parents[n]->selectedConfigurations[query] = -1;		// Backtrack, do not serve this query
				}
			}

			// Fill the rest of the queries in order, using the same technique
			for (int i = 0; i < algorithm.problemInstance.nQueries; i++) 
			{
				if (parents[n]->selectedConfigurations[i] < 0) 
				{
					parents[n]->selectedConfigurations[i] = maxGainGivenQuery(i);
					usedConfigs.emplace_back(parents[n]->selectedConfigurations[i]);

					if (parents[n]->memoryCost() > algorithm.problemInstance.M) 
					{
						usedConfigs.pop_back();
						if (i % 3 == 2) parents[n]->selectedConfigurations[i] = getHighestGainConfiguration(usedConfigs, i);
						if (i % 3 == 1) parents[n]->selectedConfigurations[i] = getRandomConfiguration(usedConfigs, i);
						else parents[n]->selectedConfigurations[i] = -1;
					}
				}
			}

			parents[n]->evaluate();		// Evaluation of the new solution
		}

		break;
	}

	case 1:
	{
		std::vector<int> b = std::vector<int>(algorithm.problemInstance.nIndexes, 0);

		for (int n = 1; n < POPULATION_SIZE; n++) 
		{
			std::fill(b.begin(), b.end(), 0);
			parents[n] = new Solution(algorithm.problemInstance);

			// Examine each query in order
			for (int i = 0, mem = 0; i < algorithm.problemInstance.nQueries; i++)
			{
				if (parents[n]->selectedConfigurations[i] < 0) 
				{
					int delta_mem = 0;

					// Get a random configuration that serves the current query
					int conf = algorithm.problemInstance.configServingQueries[i]
						[random_number() % algorithm.problemInstance.configServingQueries[i].size()];

					// Compute the additional memory cost required for activating that configuration
					for (int j = 0; j < algorithm.problemInstance.nIndexes; j++) {
						if (algorithm.problemInstance.configIndexesMatrix[conf][j] && b[j] == 0)
							delta_mem += algorithm.problemInstance.indexesMemoryOccupation[j];
					}

					mem += delta_mem;	// Update solution memory cost

					if (mem < algorithm.problemInstance.M)		// If the solution is still feasible...
					{
						// ... activate the indexes required by that configuration
						for (int j = 0; j < algorithm.problemInstance.nIndexes; j++) {
							if (algorithm.problemInstance.configIndexesMatrix[conf][j])
								b[j] = 1;
						}
						// Use this configuration for all unserved queries that benefit from it
						for (int k = 0; k < algorithm.problemInstance.queriesWithGain[conf].size(); k++) {
							if (parents[n]->selectedConfigurations[algorithm.problemInstance.queriesWithGain[conf][k]] == -1)
								parents[n]->selectedConfigurations[algorithm.problemInstance.queriesWithGain[conf][k]] = conf;
						}
					}
					else mem -= delta_mem;		//  Backtrack, do not use this configuration for this query
				}
			}

			parents[n]->evaluate();		// Evaluation of the new solution
		}

		break;
	}

	default:
	{
		// Initialize the entire population with default solutions
		for (int k = 1; k < POPULATION_SIZE; k++)
			parents[k] = new Solution(algorithm.problemInstance);
		break;
	}
	}

	// Initialization of the starting population set
	population.clear();
	population.insert(parents, parents + POPULATION_SIZE);

	checkImprovingSolutions(parents, POPULATION_SIZE);
}


void Genetic::GeneticThread::breedPopulation()
{
	// Select the best POPULATION_SIZE elements to use as parents
	auto it = population.begin();
	for (int i = 0; it != population.end(); ++it, ++i)
	{
		if (i < POPULATION_SIZE)
			parents[i] = *it;
		else delete *it;
	}

	// Duplicate parents before breeding, to create offsprings
	for (int i = 0; i < POPULATION_SIZE; i++) {
		offsprings[i] = new Solution(*parents[i]);
	}
	
	// Randomize the number of crossover points
	int N = (random_number() % 4) + MIN_CROSSOVER_POINTS;

	// Apply the crossover operator on pairs of solutions
	for (int i = 0; i < POPULATION_SIZE / 2; i++) {
		int A = random_number() % POPULATION_SIZE;
		int B = random_number() % POPULATION_SIZE;
		crossover(offsprings[A], offsprings[B], N);
	}

	// Apply the mutation operator on all offsprings
	for (int i = 0; i < POPULATION_SIZE ; i++) {
		mutate(offsprings[i]);
	}
}


void Genetic::GeneticThread::crossover(Solution* itemA, Solution* itemB, int N)
{
	// N-point crossover implementation: 
	// the solution vector is split into N sections of size M
	int length = algorithm.problemInstance.nQueries;
	int M = length / N;

	// Odd chromosomes are swapped between the 2 solutions
	for (int i = 0; i < length; i += 2 * M)
	{
		for (int j = 0; j < M && (j + i) < length; j++)
		{
			// Swap the genes inside the selected chromosomes
			swap(itemA->selectedConfigurations[i + j], itemB->selectedConfigurations[i + j]);
		}
	}
}


void Genetic::GeneticThread::mutate(Solution* sol)
{
	// Iterate over the genes in the solution
	for (int i = 0; i < algorithm.problemInstance.nQueries; i++)
	{
		// Mutation of the gene occurs with a probability of 1/#genes
		if (random_number() % algorithm.problemInstance.nQueries == 0)
		{
			// Chance of choosing another config that servers this query
			if (random_number() % 100 < MUTATION_PROBABILITY_NONZERO) {
				short int randomConfigIndex = random_number() % algorithm.problemInstance.configServingQueries[i].size();
				sol->selectedConfigurations[i] = algorithm.problemInstance.configServingQueries[i][randomConfigIndex];
			}
			// Chance of resetting this query to being served by "no configuration"
			else sol->selectedConfigurations[i] = -1;
		}
	}
}


bool Genetic::GeneticThread::replacePopulationByFitness()
{
	// Evaluate the generated offsprings one by one
	for (int i = 0; i < POPULATION_SIZE; i++)
		offsprings[i]->evaluate();

	// Replace the old population with the current parents and offsprings
	population.clear();
	population.insert(parents, parents + POPULATION_SIZE);
	population.insert(offsprings, offsprings + POPULATION_SIZE);

	return checkImprovingSolutions(offsprings, POPULATION_SIZE);
}


bool Genetic::GeneticThread::checkImprovingSolutions(Solution* candidates[], int size)
{
	Solution* best = &localBestSolution;

	// Check if there's a better solution in the candidates than the current best
	for (int i = 0; i < size; i++)
	{
		if (candidates[i]->getObjFunctionValue() > best->getObjFunctionValue())
		{
			best = candidates[i];
		}
	}

	if (best != &localBestSolution)
	{
		localBestSolution = Solution(*best);		// Update the best solution found in the current run
		return true;
	}
	
	return false;
}


void Genetic::GeneticThread::localSearch(LocalSearch& refiner)
{
	auto it = population.begin();
	std::vector<Solution*> solutions;
	solutions.reserve(POPULATION_SIZE);

	for (int i = 0; it != population.end(); ++it, i++)
	{
		if (i < POPULATION_SIZE)
		{
			// Run local-search improvement on each solution
			refiner.setStartingPoint(**it);
			solutions.push_back(new Solution(refiner.run(*algorithm.parameters)));
		}
		delete* it;
	}

	// Replace the previous individuals in the population
	// with their improved counterparts
	population.clear();
	population.insert(solutions.begin(), solutions.end());
}


/* ============================================================================== */
/* ============================================================================== */


// Pick a random configuration (that provides a gain > 0) out of those already used by a solution
int Genetic::GeneticThread::getRandomConfiguration(std::vector<int>& usedConfigs, int queryIndex)
{
	for (int i = 0; i < usedConfigs.size(); i++) {
		int randomConfig = usedConfigs[random_number() % usedConfigs.size()];
		if (algorithm.problemInstance.configQueriesGain[randomConfig][queryIndex] > 0) {
			return randomConfig;
		}
	}

	return -1;	// if none, backtrack
}


// Given the configurations already used by a solution, 
// pick one that gives the highest gain to a specific query
int Genetic::GeneticThread::getHighestGainConfiguration(std::vector<int>& usedConfigs, int queryIndex)
{
	int maxConfig = -1;

	for (int i = 0, maxGain = 0; i < usedConfigs.size(); i++) {
		if (algorithm.problemInstance.configQueriesGain[usedConfigs[i]][queryIndex] > maxGain) 
		{
			maxGain = algorithm.problemInstance.configQueriesGain[usedConfigs[i]][queryIndex];
			maxConfig = usedConfigs[i];
		}
	}

	return maxConfig;
}


// Returns the index of the configuration providing the maximum gain for a given query
int Genetic::GeneticThread::maxGainGivenQuery(int queryIndex)
{
	int maxConfig = -1;
	const Instance& inst = algorithm.problemInstance;
	
	for (int i = 0, maxGain = 0; i < inst.configServingQueries[queryIndex].size(); i++) {
		if (inst.configQueriesGain[inst.configServingQueries[queryIndex][i]][queryIndex] > maxGain) 
		{
			maxGain = inst.configQueriesGain[inst.configServingQueries[queryIndex][i]][queryIndex];
			maxConfig = inst.configServingQueries[queryIndex][i];
		}
	}

	return maxConfig;
}
