#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <string>

#define DEFAULT_TIMELIMIT 300


/* ============ DATA TYPES ============ */


// Wrapper structure used to hold and pass the command line execution parameters
typedef struct
{
	std::string inputFileName = std::string();
	std::string outputFileName = std::string();		// Not a parameter, can be generated as <inputFileName>_OMAAL_group04.sol
	unsigned int timeLimit = DEFAULT_TIMELIMIT;
	bool parsingError = false;
} Params;



// Data structure used to hold the input dataset of the problem instance
typedef struct
{
	unsigned int nQueries;		// |Q|
	unsigned int nIndexes;		// |I|
	unsigned int nConfigs;		// |C|
	unsigned int M;
	short int **configIndexesMatrix;		// e matrix
	unsigned int *indexesFixedCost;			// f vector
	unsigned int *indexesMemoryOccupation;	// m vector
	unsigned int **configQueriesGain;		// g matrix
} Instance;



/* ========== CLASSES ========== */

// A solution is represented by a class because it holds both data and functions
class Solution
{

	// --- DATA ---

public:

	short int **configsServingQueries;			// x matrix
	short int *indexesToBuild;					// b vector
	unsigned long int objFunctionValue = -1;
	

private:

	Instance *problemInstance;	// TODO: initialize these values inside the Solution class constructor
	
	// --- METHODS ---

public:

	bool isFeasible();
	unsigned long int evaluateObjectiveFunction();
	void writeToFile(std::string fileName);

private:

};



/* ============ FUNCTIONS ============ */

Params parseCommandLine (int argc, char* argv[]);
Instance readInputFile (std::string fileName);


#endif // UTILITIES_HPP
