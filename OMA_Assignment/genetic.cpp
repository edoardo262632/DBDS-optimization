#include "genetic.hpp"
#include <chrono>

using namespace std::chrono;


void Genetic::run(const Instance & problemInstance, const Params & parameters)
{
	long long startingTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

	// INITIALIZATION
	initializePopulation(POPULATION_SIZE);
	long long currentTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

	// REPEAT UNTIL THERE'S COMPUTATIONAL TIME LEFT
	while (currentTime - startingTime < parameters.timeLimit)
	{
		breedPopulation(POPULATION_SIZE);
		evaluateFitness(POPULATION_SIZE);
		replacePopulation(POPULATION_SIZE);
			
		currentTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();		// update timestamp
	}
}



void Genetic::initializePopulation(int size)
{
	parents[0] = Solution(&problemInstance);		// one solution is kept with the default configuration

	for (int n = 1; n < size; n++)					// P-1 solutions are initialized with the greedy algorithm
	{
		parents[n] = Solution(&problemInstance);
		for (unsigned int i = 0; i < problemInstance.nQueries; i++) {

			int A = rand() % problemInstance.nConfigs;		// generate a random value for the configuration to take for each query
			parents[n].configsServingQueries[A][i] = 1;
			if (memoryCost(problemInstance, parents[n]) > problemInstance.M)
				parents[n].configsServingQueries[A][i] = 0;						// "backtrack" -> do not activate this configuration
		}
	}
}



void Genetic::breedPopulation(int size)
{
	// ITERATE OVER THE POPULATION SORTED SET (using c++ collection iterator)
	// AND COPY THE FIRST size SOLUTIONS (pointers) INTO THE parents ARRAY

	// COMBINE (CALLING crossover() FUNCTION) THE FIRST PARENT WITH THE LAST IN THE ARRAY, THE SECOND WITH THE LAST BUT ONE, ...
	// FILLING THE offsprings ARRAY WITH THE NEW SOLUTIONS 
	// (you will need to create 2 new solutions by copying the parents before calling the crossover function,
	// because we don't want to lose the parents, DO NOT copy the content of the solutions manually 
	// but use the copy constructor Solution(Solution& other) instead)

	// ITERATE OVER THE OFFSPRINGS ARRAY AND CALL THE mutate() FUNCTION ON ALL OF THEM
}


void Genetic::evaluateFitness(int size)
{
	// ITERATE OVER THE OFFSPRINGS ARRAY AND CALL THE EVALUATE FUNCTION ON ALL OF THEM
	// IF A SOLUTION HAS A BETTER OBJ.FUNCTION VALUE THAN THE CURRENT bestSolution, REPLACE IT

	// The evaluation is done through the evaluate() method in Solution class, which has yet to be implemented but
	// it's actually a merge between the already existing isFeasible() and evaluateObjectiveFunction() methods
}


void Genetic::replacePopulation(int size)
{
	// CALL THE clear() METHOD ON THE POPULATION SET, THEN
	// ITERATE OVER THE PARENTS ARRAY AND ALL OF THEM TO THE POPULATION SET WITH THE insert() METHOD
	// THEN ITERATE OVER ALL THE OFFSPRINGS AND ADD THEM TO THE POPULATION SET IN THE SAME WAY
}


void Genetic::crossover(Solution& itemA, Solution& itemB)
{
	// DO N-POINT CROSSOVER BETWEEN THE 2 SOLUTIONS
	// both the integer array values (index of the configuration serving a query) and the relative column
	// in the matrix have to be moved when doing the crossover, for moving columns ONLY SWAP POINTERS
}


void Genetic::mutate(Solution & sol)
{
	// FOR EACH ITEM IN THE INTEGER ARRAY (index of the configuration serving a query) DO:
		// EXTRACT A RANDOM NUMBER, IF IT SATISFIES A CONDITION WHICH HAS A 1/nQueries PROBABILITY THE GENE IS MUTATED, WHICH MEANS
		// EXTRACTING A RANDOM NUMBER WITH A 50% CHANCE OF SETTING THE GENE TO 0 AND A 50% CHANCE OF SETTING ITS VALUE TO THE INDEX
		// OF ANOTHER CONFIGURATION SERVING THAT QUERY (an appropriate data structure for this will be added to the problemInstance struct)
}