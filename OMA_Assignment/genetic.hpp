#ifndef GREEDY_HPP
#define GREEDY_HPP

#include "algorithm.hpp"
#include "utilities.hpp"
#include <set>

#define POPULATION_SIZE 10
#define DETERMINISTIC_RANDOM_NUMBER_GENERATION false

class Genetic : Algorithm
{
	// ====== DATA ======

public:

private:

	// Comparator for Solution objects, to use in the population sorted set
	struct solution_comparator {
		bool operator() (Solution& lhs, Solution& rhs) const
		{
			return lhs.evaluate() < rhs.evaluate();
		}
	};

	Solution* parentsCopy;
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
		offsprings = (Solution*)malloc((2 * POPULATION_SIZE - 1) * sizeof(Solution));
	}

	void run(const Instance& problemInstance, const Params& parameters);

private:

	void initializePopulation(int size);
	void breedPopulation(int size);
	void evaluateFitness(int size);
	void replacePopulation(int size);

	void crossover(Solution& itemA, Solution& itemB);
	void mutate(Solution& sol);
};

#endif	// GENETIC_HPP

