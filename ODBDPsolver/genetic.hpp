#pragma once

#include <set>
#include <vector> 
#include <thread>  
#include <mutex>
#include <random>

#include "algorithm.hpp"
#include "localsearch.hpp"


#define MIN_CROSSOVER_POINTS 2
#define POPULATION_SIZE 100
#define MUTATION_PROBABILITY_NONZERO 90
#define N_THREADS 2

using namespace std;


class Genetic : public Algorithm
{
	
	// Worker thread class for running multiple instances of the algorithm in parallel
	class GeneticThread
	{

		// Comparator for Solution objects, to use in the population sorted set
		struct solution_comparator {
			bool operator() (const Solution* lhs, const Solution* rhs) const
			{
				return lhs->getFitnessValue() > rhs->getFitnessValue();
			}
		};

	private:

		const int threadID;
		Genetic& algorithm;
		Solution localBestSolution;
		Solution* parents[POPULATION_SIZE] = { nullptr };
		Solution* offsprings[POPULATION_SIZE] = { nullptr };
		std::multiset<Solution*, solution_comparator> population;
		std::mt19937 random_number;

		unsigned int generation_counter;
		unsigned int MAX_GENERATIONS_BEFORE_RESTART = 1000;


	public:

		GeneticThread(Genetic& caller, int tID);
		~GeneticThread();

		void run();		// Thread entry point

	private:

		// Genetic algorithm steps, implemented each by a function
		void initializePopulation();
		void initializePopulation2();
		void breedPopulation();
		void crossover(Solution* itemA, Solution* itemB, int N = 2);
		void mutate(Solution* sol);
		bool replacePopulationByFitness();
		bool checkImprovingSolutions(Solution* candidates[], int size);
		void localSearch(LocalSearch& refiner);

		// Auxiliary functions for greedy initialization
		int getRandomConfiguration(std::vector<int>& usedConfigs, int queryIndex);
		int getHighestGainConfiguration(std::vector<int>& usedConfigs, int queryIndex);
		int maxGainGivenQuery(int queryIndex);
	};



private:

	const Parameters* parameters;
	vector<GeneticThread> threads;
	mutex mtx;
	

public:

	Genetic(Instance& inst);
	~Genetic();

	Solution run(const Parameters& parameters);

private:

	void updateBestSolution(const Solution& newBest);

};
