#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP

#include "utilities.hpp"

/*
** Algorithm provides a generic interface for an algorithm's implementation,
** in order to achieve modularity and freedom of exchanging the algorithm in use for testing and tweaking purposes
** Members of this class (data or functions) must be shared by ALL possible algorithm implementations
*/
class Algorithm
{
	// ====== DATA ======

public:

protected:

	Solution bestSolution;

private:

	// ====== METHODS ======

public:

	Algorithm(Instance& inst) : bestSolution(&inst) {}

	virtual void run(const Instance& problemInstance, unsigned int time) = 0;

private:

};


#endif // ALGORITHM_HPP