#ifndef GENETIC_HPP
#define GENETIC_HPP

#include "algorithm.hpp"
#include <set>
#include <vector> 

#define MIN_CROSSOVER_POINTS 2
#define MAX_GENERATIONS_BEFORE_RESTART 10000
#define DETERMINISTIC_RANDOM_NUMBER_GENERATION false

class Genetic : Algorithm
{
	// ====== DATA ======

public:

private:

	// Comparator for Solution objects, to use in the population sorted set
	struct solution_comparator {
		bool operator() (const Solution* lhs, const Solution* rhs) const
		{
			return lhs->getObjFunctionValue() > rhs->getObjFunctionValue();
		}
	};

	unsigned int POPULATION_SIZE;
	Solution** parents;
	Solution** offsprings;
	std::multiset<Solution*, solution_comparator> population;

	// ====== METHODS ======

public:

	Genetic(Instance* inst)
		: Algorithm(inst),		// base class constructor
		population(std::multiset<Solution*, solution_comparator>())
	{
		POPULATION_SIZE = inst->nQueries;
		parents = (Solution**)malloc(POPULATION_SIZE * sizeof(Solution*));
		offsprings = (Solution**)malloc(POPULATION_SIZE * sizeof(Solution*));
	}

	Solution* run(const Params& parameters);

private:

	void initializePopulation();
	void breedPopulation();
	void logPopulation();

	bool evaluateFitness(const std::string outputFileName, unsigned int gen);
	void replacePopulation();

	void crossover(Solution* itemA, Solution* itemB, unsigned int N = 2);
	void mutate(Solution* sol);

	// tmp
	Solution* generateRandomSolution();

	// extra functions to initialize
	void initializePopulation2(int size);
	void initializePopulation3(int size);
	void initializePopulationGreedy(int size);

	// auxiliary functions to set configurations for queries during initialization
	int getRandomConfiguration(std::vector<int> usedConfigs, int queryIndex);
	int getHighestGainConfiguration(std::vector<int> usedConfigs, int queryIndex);
	int maxGainGivenQuery(int queryIndex);

};

#endif	// GENETIC_HPP

