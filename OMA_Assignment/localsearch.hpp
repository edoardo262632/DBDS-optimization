#ifndef LOCALSEARCH_HPP
#define LOCALSEARCH_HPP

#include "algorithm.hpp"


class LocalSearch : Algorithm
{
	// ====== DATA ======

public:

private:

	Solution* startingPoint;

	// ====== METHODS ======

public:

	LocalSearch(Instance* inst)
		: Algorithm(inst),		// base class constructor
		startingPoint(new Solution(bestSolution))
	{ }

	void setStartingPoint(Solution* sol);
	Solution* run(const Params& parameters);

private:


};

#endif	// LOCALSEARCH_HPP
