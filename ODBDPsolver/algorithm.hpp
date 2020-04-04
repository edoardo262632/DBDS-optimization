#pragma once

#include "utilities.hpp"


/*
** Algorithm provides a generic interface for an algorithm's implementation,
** this gives us modularity and freedom of exchanging the algorithm in use for testing and tweaking purposes
** Members of this class (data or functions) must be shared by ALL possible algorithm implementations
*/
class Algorithm
{

protected:

	Instance& problemInstance;
	Solution bestSolution;

public:

	Algorithm(Instance& inst)
		: problemInstance(inst),
		bestSolution(Solution(inst))
	{ };

	virtual Solution run(const Parameters& parameters) = 0;

private:

};
