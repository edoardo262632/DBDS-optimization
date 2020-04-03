#pragma once

#include <string>
#include <vector>

#define DEFAULT_TIMELIMIT 180*1000	// ms

using namespace std;


typedef struct Params		// Wrapper structure used to hold command line execution parameters
{
	string inputFileName = string();
	string outputFileName = string();				// Generated as <inputFileName>_OMAAL_group04.sol
	unsigned int timeLimit = DEFAULT_TIMELIMIT;
} Parameters;


/* ============ FUNCTIONS ============ */


Parameters parseCommandLine(int argc, char* argv[]);
long long getCurrentTime_ms();


/* ============= CLASSES ============= */


class Instance		// Holds the input dataset of the problem instance
{

public:

	int nQueries;		// |Q|
	int nIndexes;		// |I|
	int nConfigs;		// |C|
	int M;				// Memory

	vector<vector<short>> configIndexesMatrix;	 // e matrix
	vector<int> indexesFixedCost;				 // f vector
	vector<int> indexesMemoryOccupation;		 // m vector
	vector<vector<int>> configQueriesGain;		 // g matrix

	vector<vector<int>> configServingQueries;	 // #Queries vectors  
	vector<vector<int>> queriesWithGain;		 // #Configuration vectors


public:

	Instance();
	~Instance();

	void readInputFile(const std::string& fileName);	// Instance input

};



class Solution		// Represent a possible solution for the given problem
{

public:

	vector<short> selectedConfigurations;		// Compact integer representation of the solution matrix

private:

	vector<short> indexesToBuild;			// b vector
	Instance& problemInstance;
		
	long objFunctionValue;
	long fitnessValue;


public:

	Solution(Instance& probInst);					// Constructs an empty feasible Solution for the problem Instance
	Solution(const Solution& other);				// Copy constructor
	Solution& operator=(const Solution& other);		// Copy-assignment operator
	~Solution();

	long evaluate();
	int memoryCost();
	long getObjFunctionValue() const;
	long getFitnessValue() const;

	void writeToFile(const std::string& fileName) const;	// Solution output
	
};
