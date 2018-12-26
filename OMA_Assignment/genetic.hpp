#ifndef GENETIC_HPP
#define GENETIC_HPP

#include "algorithm.hpp"
#include "localsearch.hpp"
#include <set>
#include <vector> 

#define MIN_CROSSOVER_POINTS 2

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
			return lhs->getFitnessValue() > rhs->getFitnessValue();
		}
	};

	unsigned int MAX_GENERATIONS_BEFORE_RESTART = 10000;
	unsigned int POPULATION_SIZE;
	Solution** parents;
	Solution** offsprings;
	std::multiset<Solution*, solution_comparator> population;
	unsigned int generation_counter;
	std::string outputFileName;
	
	// ====== METHODS ======

public:

	Genetic(Instance* inst)
		: Algorithm(inst),		// base class constructor
		population(std::multiset<Solution*, solution_comparator>())
	{
		POPULATION_SIZE = 2 * inst->nQueries;
		parents = (Solution**)malloc(POPULATION_SIZE * sizeof(Solution*));
		offsprings = (Solution**)malloc(POPULATION_SIZE * sizeof(Solution*));
	}

	Solution* run(const Params& parameters);

private:

	void initializePopulation();
	void breedPopulation();
	void logPopulation();
	bool replacePopulationByFitness(const std::string outputFileName, unsigned int gen);
	void replaceLowerHalfPopulation();
	bool checkImprovingSolutions(Solution** candidates, int size);

	void crossover(Solution* itemA, Solution* itemB, unsigned int N = 2);
	void mutate(Solution* sol);
	void localSearch(LocalSearch* refiner, const Params& parameters);

	// auxiliary functions for greedy initialization
	int getRandomConfiguration(std::vector<int> usedConfigs, int queryIndex);
	int getHighestGainConfiguration(std::vector<int> usedConfigs, int queryIndex);
	int maxGainGivenQuery(int queryIndex);
	Solution* generateRandomSolution();
	void initializePopulation2();
};

#endif	// GENETIC_HPP
