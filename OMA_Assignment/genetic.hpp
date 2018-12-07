#ifndef GREEDY_HPP
#define GREEDY_HPP

#include "algorithm.hpp"
#include "utilities.hpp"
#include <set>

#define POPULATION_SIZE 10

class Genetic : Algorithm
{
	// ====== DATA ======

public:

private:

	Solution* parents;
	Solution* offsprings;
	std::set<Solution> population;

	// ====== METHODS ======

public:

	Genetic(Instance& inst)
		: Algorithm(inst)		// base class (Algorithm) constructor
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

