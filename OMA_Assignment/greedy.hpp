#ifndef GREEDY_HPP
#define GREEDY_HPP

#include "algorithm.hpp"
#include "utilities.hpp"

class Greedy : Algorithm
{
	// ====== DATA ======

public:

private:

	// ====== METHODS ======
	
public:

	void run(const Instance& problemInstance, unsigned int time);

private:

	unsigned int memoryCost(const Instance& problemInstance, const Solution& solution);
};
	
#endif	// GREEDY_HPP


