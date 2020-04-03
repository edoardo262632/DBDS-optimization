#pragma once

#include "algorithm.hpp"


class LocalSearch : public Algorithm
{

private:

	Solution startingPoint;


public:

	LocalSearch(Instance& inst)
		: Algorithm(inst),
		startingPoint(Solution(bestSolution))
	{ };

	void setStartingPoint(const Solution& sol);
	Solution run(const Parameters& parameters);

private:

};
