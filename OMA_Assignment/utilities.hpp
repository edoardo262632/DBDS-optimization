#ifndef UTILITIES_HPP
#define UTILITIES_HPP
#include <string>
#include <iostream>
#include <limits>

#define DEFAULT_TIMELIMIT 300*1000


/* ============ DATA TYPES ============ */


// Wrapper structure used to hold and pass the command line execution parameters
typedef struct
{
	std::string inputFileName = std::string();
	std::string outputFileName = std::string();		// Not a parameter, can be generated as <inputFileName>_OMAAL_group04.sol
	unsigned int timeLimit = DEFAULT_TIMELIMIT;
	bool parsingError = false;
} Params;

// Data structure that holds for each query the used configuration
typedef struct
{
	unsigned int* config;
	unsigned int lenght;
} UsefulConfig;

// Data structure used to hold the input dataset of the problem instance
typedef struct
{
	unsigned int nQueries;		// |Q|
	unsigned int nIndexes;		// |I|
	unsigned int nConfigs;		// |C|
	unsigned int M;							// memory
	short int **configIndexesMatrix;		// e matrix
	unsigned int *indexesFixedCost;			// f vector
	unsigned int *indexesMemoryOccupation;	// m vector
	unsigned int **configQueriesGain;		// g matrix
	UsefulConfig *configServingQueries;
} Instance;



/* ========== CLASSES ========== */

// A solution is represented by a class because it holds both data and functions
class Solution
{

	// --- DATA ---

public:

	short int **configsServingQueries;			// x matrix
	short int *indexesToBuild;					// b vector
	short int *selectedConfiguration;			// compact integer representation of the solution matrix
	

private:

	long int objFunctionValue;
	Instance *problemInstance;	// TODO: initialize these values inside the Solution class constructor
	
	// --- METHODS ---

public:

	Solution(Instance *probInst);		// constructor of an empty feasible Solution object for a problem Instance
	Solution(const Solution& other);	// copy constructor

	long int evaluate();
	void writeToFile(const std::string fileName) const;

	bool isFeasible();
	long int evaluateObjectiveFunction();
	
private:


};



/* ============ FUNCTIONS ============ */

Params parseCommandLine (int argc, char* argv[]);
Instance readInputFile (std::string fileName);
unsigned int memoryCost(const Instance& problemInstance, const Solution& solution);

#endif // UTILITIES_HPP
