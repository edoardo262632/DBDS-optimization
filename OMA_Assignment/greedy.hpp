#ifndef GREEDY_HPP
#define GREEDY_HPP
#include "algorithm.hpp"
#include "utilities.hpp"

class Greedy:Algorithm
{
	// ====== DATA ======

public:

private:

	// ====== METHODS ======
	Solution bestSolution;
	

public:

	void run(Instance problemInstance, unsigned int time);


private:
		bool check(Instance problemInstance);
};
	


#endif


