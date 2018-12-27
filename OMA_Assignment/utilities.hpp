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

// Custom general-purpose array data structure
typedef struct {
	int* vector;
	int length;
} CustomArray;

// Data structure used to hold the input dataset of the problem instance
typedef struct
{
	int nQueries;		// |Q|
	int nIndexes;		// |I|
	int nConfigs;		// |C|
	int M;								 // memory
	short int **configIndexesMatrix;	 // e matrix
	int *indexesFixedCost;				 // f vector
	int *indexesMemoryOccupation;		 // m vector
	int **configQueriesGain;			 // g matrix
	CustomArray *queriesWithGain;		 // #Configuration vectors  
	CustomArray *configServingQueries;	 // #Queries vectors  
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
	long int fitnessValue;


	// --- METHODS ---

public:

	Solution(Instance *probInst);		// constructor of an empty feasible Solution object for a problem Instance
	Solution(Solution* other);			// copy constructor
	~Solution();						// solution class destructor

	long int evaluate();
	int memoryCost();
	long int getObjFunctionValue() const;
	long int getFitnessValue() const;
	void writeToFile(const std::string fileName) const;
	void print(const std::string msg) const;
	
private:


};



/* ============ FUNCTIONS ============ */

Params parseCommandLine (int argc, char* argv[]);
Instance readInputFile (std::string fileName);
long long getCurrentTime_ms();

#endif // UTILITIES_HPP
