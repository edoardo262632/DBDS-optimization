#ifndef GREEDY_HPP
#define GREEDY_HPP

#include "algorithm.hpp"

class Greedy : Algorithm
{
	// ====== DATA ======

public:

private:

	// ====== METHODS ======
	
public:

	Greedy(Instance& inst)
		: Algorithm(inst)		// base class (Algorithm) constructor
	{
	}

	void run(const Instance& problemInstance, const Params& parameters);

private:

};
	
#endif	// GREEDY_HPP


