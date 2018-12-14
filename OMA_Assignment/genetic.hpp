#ifndef GENETIC_HPP
#define GENETIC_HPP

#include "algorithm.hpp"
#include <set>

#define POPULATION_SIZE 32
#define N_CROSSOVER_POINTS 4
#define DETERMINISTIC_RANDOM_NUMBER_GENERATION true

class Genetic : Algorithm
{
	// ====== DATA ======

public:

private:

	// Comparator for Solution objects, to use in the population sorted set
	struct solution_comparator {
		bool operator() (const Solution& lhs, const Solution& rhs) const
		{
			return lhs.getObjFunctionValue() < rhs.getObjFunctionValue();
		}
	};

	Solution* parents;
	Solution* offsprings;
	std::set<Solution, solution_comparator> population;

	// ====== METHODS ======

public:

	Genetic(Instance& inst)
		: Algorithm(inst),		// base class constructor
		population(std::set<Solution, solution_comparator>())
	{
		parents = (Solution*)malloc(POPULATION_SIZE * sizeof(Solution));
		offsprings = (Solution*)malloc(POPULATION_SIZE * sizeof(Solution));
	}

	void run(const Instance& problemInstance, const Params& parameters);

private:

	void initializePopulation(int size);
	void breedPopulation(int size);
	void evaluateFitness(int size, const std::string outputFileName);
	void replacePopulation(int size);

	void crossover(Solution& itemA, Solution& itemB, unsigned int N = 2);
	void mutate(Solution& sol);
};

#endif	// GENETIC_HPP

