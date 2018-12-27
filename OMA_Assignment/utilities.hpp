#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <string>
#include <limits>
#include <chrono>

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

// Data structure that holds for each query the configurations with a non-zero gain
typedef struct
{
	unsigned int* configs;
	unsigned int length;
} UsefulConfigs;

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
	UsefulConfigs *queriesWithGain;			// #Configuration vectors  
	UsefulConfigs *configServingQueries;	// #Queries vectors  
	float avgGainPerMemoryUnit;
} Instance;



/* ========== CLASSES ========== */

class Solution
{

	// --- DATA ---

public:

	short int *selectedConfiguration;			// compact integer representation of the solution matrix

private:

	short int *indexesToBuild;					// b vector
	Instance *problemInstance;
		
	long int objFunctionValue;
	float fitnessValue;


	// --- METHODS ---

public:

	Solution(Instance *probInst);		// constructor of an empty feasible Solution object for a problem Instance
	Solution(Solution* other);			// copy constructor
	~Solution();						// solution class destructor

	long int evaluate();
	unsigned int memoryCost();
	long int getObjFunctionValue() const;
	long int getFitnessValue() const;
	void writeToFile(const std::string fileName) const;
	
private:


};



/* ============ FUNCTIONS ============ */

Params parseCommandLine (int argc, char* argv[]);
Instance readInputFile (std::string fileName);
long long getCurrentTime_ms();

#endif // UTILITIES_HPP
