#ifndef GENETIC_HPP
#define GENETIC_HPP

#include "algorithm.hpp"
#include "localsearch.hpp"
#include <set>
#include <vector> 
#include <thread>  
#include <mutex>
#include <random>

#define MIN_CROSSOVER_POINTS 2
#define POPULATION_SIZE_MULTIPLIER 2
#define MUTATION_PROBABILITY_NONZERO 90
#define N_THREADS 4

class Genetic : Algorithm
{
	// ====== TYPES ======

	// Comparator for Solution objects, to use in the population sorted set
	struct solution_comparator {
		bool operator() (const Solution* lhs, const Solution* rhs) const
		{
			return lhs->getFitnessValue() > rhs->getFitnessValue();
		}
	};

	// Thread worker class for running multiple instances of the algorithm in parallel
	class GeneticThread
	{
		// ====== DATA ======

	public:

	private:

		const int threadID;
		Genetic* algorithm;
		Solution* localBestSolution;
		Solution** parents;
		Solution** offsprings;
		std::multiset<Solution*, solution_comparator> population;

		const int POPULATION_SIZE;
		unsigned int generation_counter;
		unsigned int MAX_GENERATIONS_BEFORE_RESTART = 1000;

		// ====== METHODS ======

	public:

		GeneticThread(Genetic* caller, int tID)
			: algorithm(caller), threadID(tID),
			population(std::multiset<Solution*, solution_comparator>()),
			localBestSolution(new Solution(algorithm->problemInstance)),
			POPULATION_SIZE(POPULATION_SIZE_MULTIPLIER * algorithm->problemInstance->nQueries)
		{
			parents = (Solution**)malloc(POPULATION_SIZE * sizeof(Solution*));
			offsprings = (Solution**)malloc(POPULATION_SIZE * sizeof(Solution*));
		}

		~GeneticThread()
		{
			free(parents);
			free(offsprings);
		}

		void run();		// Worker thread main function


	private:

		void initializePopulation();
		void initializePopulation2();
		void breedPopulation();
		bool replacePopulationByFitness();
		bool checkImprovingSolutions(Solution** candidates, int size);

		void crossover(Solution* itemA, Solution* itemB, int N = 2);
		void mutate(Solution* sol);
		void localSearch(LocalSearch* refiner);

		// Auxiliary functions for greedy initialization
		int getRandomConfiguration(std::vector<int> usedConfigs, int queryIndex);
		int getHighestGainConfiguration(std::vector<int> usedConfigs, int queryIndex);
		int maxGainGivenQuery(int queryIndex);

		// Auxiliary function for randomization
		int intRand(int max);
	};


	// ====== DATA ======

public:

private:

	const Params* parameters;
	GeneticThread** threadss;
	

	// ====== METHODS ======

public:

	Genetic(Instance* inst)
		: Algorithm(inst)		// base class constructor
	{
		threadss = (GeneticThread**)malloc(N_THREADS * sizeof(GeneticThread*));
		for (int i = 0; i < N_THREADS; i++)
			threadss[i] = new GeneticThread(this, (i+1));		// instantiate thread worker classes
	}

	Solution* run(const Params* parameters);


private:

	void updateBestSolution(Solution* newBest);

};

#endif	// GENETIC_HPP
