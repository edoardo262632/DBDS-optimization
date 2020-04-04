#include "utilities.hpp"

#include <limits>
#include <chrono>
#include <exception>


Parameters parseCommandLine(int argc, char *argv[])
{	
	Parameters execParams = Params();

	if (argc != 2 && argc != 4)
	{
		throw exception("Wrong command line format, expected:\
			\n$ODBDPsolver_OMAAL_group04.exe <instancefilename> -t <timelimit>");
	}
	else
	{
		for (int i = 1; i < argc; ++i)
		{
			// Parsing the -t <timelimit> parameter, which takes up 2 consecutive args
			if (strcmp(argv[i], "-t") == 0 && i < argc-1)		
			{
				execParams.timeLimit = (unsigned) atoi(argv[i + 1])*1000;
				i++;
			}
			// Parsing the <inputfilename> parameter, also generating the output filename
			else if (execParams.inputFileName.length() == 0)		
			{
				execParams.inputFileName = std::string(argv[i]);
				execParams.outputFileName = execParams.inputFileName + "_OMAAL_group04.sol";
			}
			else	// Unknown parameter handling										
			{
				throw exception("Command line parsing error, expected:\
					\n$ODBDPsolver_OMAAL_group04.exe <instancefilename> -t <timelimit>");
			}
		}
	}

	return execParams;
}


long long getCurrentTime_ms()		// Returns system time in milliseconds
{
	using namespace chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}


/****	INSTANCE CLASS	****/


Instance::Instance()
	: M(0), nConfigs(0), nQueries(0), nIndexes(0)
{
}

Instance::~Instance()
{
}


void Instance::readInputFile(const std::string& fileName)
{
	FILE* fl;
	fopen_s(&fl, fileName.c_str(), "r");
	if (fl == NULL)
	{
		throw exception(("Error when attempting to open and read file '" + fileName + "'\n").c_str());
	}

	// Read problem instance size values
	int l = fscanf_s(fl, "%*s%u", &this->nQueries);
	l += fscanf_s(fl, "%*s%u", &this->nIndexes);
	l += fscanf_s(fl, "%*s%u", &this->nConfigs);
	l += fscanf_s(fl, "%*s%u", &this->M);

	if (l != 4)
		throw exception("Error in the instance file format\n");

	fscanf_s(fl, "%*s");	// Skip a row


	// Read the CONFIGURATION_INDEX_MATRIX
	configIndexesMatrix.clear();
	configIndexesMatrix.reserve(nConfigs);
	for (int i = 0; i < nConfigs; i++)
	{
		configIndexesMatrix.emplace_back(vector<short int>(nIndexes, 0));
		for (short int j = 0; j < nIndexes; j++)
			fscanf_s(fl, "%hd", &configIndexesMatrix[i][j]);
	}

	fscanf_s(fl, "%*s");	// Skip a row


	// Allocate and read fixed_cost of each index
	indexesFixedCost = vector<int>(nIndexes, 0);
	for (int i = 0; i < nIndexes; i++) {
		fscanf_s(fl, "%u", &indexesFixedCost[i]);
	}

	fscanf_s(fl, "%*s");	// Skip a row


	// Allocate and read memory_cost of each index
	indexesMemoryOccupation = vector<int>(nIndexes, 0);
	for (int i = 0; i < nIndexes; i++) {
		fscanf_s(fl, "%u", &indexesMemoryOccupation[i]);
	}

	fscanf_s(fl, "%*s");	// Skip a row


	// Allocate and read the CONFIGURATION_QUERIES_GAIN
	configQueriesGain.clear();
	configQueriesGain.reserve(nConfigs);
	configServingQueries.clear();
	configServingQueries.reserve(nQueries);
	queriesWithGain.clear();
	queriesWithGain.reserve(nConfigs);

	for (int i = 0; i < nQueries; i++)
		configServingQueries.emplace_back(std::vector<int>());

	for (int i = 0; i < nConfigs; i++)
		queriesWithGain.emplace_back(std::vector<int>());

	for (int i = 0; i < nConfigs; i++) 
	{
		configQueriesGain.emplace_back(vector<int>(nQueries, 0));
		for (int j = 0; j < nQueries; j++)
		{
			fscanf_s(fl, "%u", &configQueriesGain[i][j]);

			// Count non-zero elements
			if (configQueriesGain[i][j] > 0) {
				configServingQueries[j].emplace_back(0);
				queriesWithGain[i].emplace_back(0);
			}
		}
	}

	fclose(fl);


	// Populate the configServingQueries data structure
	for (int i = 0, k = 0; i < nQueries; i++, k = 0)
	{
		for (int j = 0, k = 0; j < nConfigs; j++) {
			if (configQueriesGain[j][i] > 0)
				configServingQueries[i][k++] = j;
		}
	}

	// Populate the queriesWithGain data structure
	for (int i = 0, k = 0; i < nConfigs; i++, k = 0) 
	{
		for (int j = 0; j < nQueries; j++) {
			if (configQueriesGain[i][j] > 0)
				queriesWithGain[i][k++] = j;
		}
	}
}


/****	SOLUTION CLASS	****/


Solution::Solution(Instance& probInst)
	: objFunctionValue(0),
	fitnessValue(0),
	memory(0),
	problemInstance(probInst),
	selectedConfigurations(vector<short>(probInst.nQueries, -1)),		// Initialize default solution
	indexesToBuild(vector<short>(probInst.nIndexes, 0))
{	
}

Solution::Solution(const Solution& other)
	: problemInstance(other.problemInstance),
	objFunctionValue(other.objFunctionValue),
	fitnessValue(other.fitnessValue),
	memory(other.memory),
	selectedConfigurations(other.selectedConfigurations),
	indexesToBuild(vector<short>(other.problemInstance.nIndexes, 0))
{
}

Solution& Solution::operator=(const Solution& other)
{
	if (this != &other)
	{
		this->problemInstance = other.problemInstance;
		this->objFunctionValue = other.objFunctionValue;
		this->fitnessValue = other.fitnessValue;
		this->memory = other.memory;
		this->selectedConfigurations = other.selectedConfigurations;
		this->indexesToBuild = vector<short>(problemInstance.nIndexes, 0);
	}

	return *this;
}

Solution::~Solution()
{
}


long int Solution::evaluate() 
{
	int all_gains = 0;
	int time_spent = 0;
	memory = 0;

	std::fill(indexesToBuild.begin(), indexesToBuild.end(), 0);

	// Calculate the gains and costs of the selected configurations
	for (int i = 0; i < problemInstance.nQueries; i++) 
	{
		if (selectedConfigurations[i] < 0)
			continue;

		for (int k = 0; k < problemInstance.nIndexes; k++) 
		{
			// If index k is part of configuration i and has not yet been built
			if (problemInstance.configIndexesMatrix[selectedConfigurations[i]][k] == 1 && indexesToBuild[k] == 0) 
			{
				indexesToBuild[k] = 1;										// Mark it as 'used'
				time_spent += problemInstance.indexesFixedCost[k];			// Add its time...
				memory += problemInstance.indexesMemoryOccupation[k];		// ...and memory cost
			}
		}
		// Add the gain of the chosen configuration for the current query
		all_gains += problemInstance.configQueriesGain[selectedConfigurations[i]][i];
	}

	// Feasibility (memory constraint)
	bool feasible = memory < problemInstance.M;

	// Objective function (total gains - index cost)
	objFunctionValue = feasible ? all_gains - time_spent : LONG_MIN;

	// Fitness function = objective function (+ penalty)
	fitnessValue = (all_gains - time_spent) -
		(feasible ? 0 : (memory - problemInstance.M));		// Penalise infeasible solutions by their surplus memory


	return objFunctionValue;
}


int Solution::evaluateMemory()
{
	memory = 0;
	std::fill(indexesToBuild.begin(), indexesToBuild.end(), 0);

	// Calculate memory cost of the solution
	for (int i = 0; i < problemInstance.nQueries; i++)
	{
		if (selectedConfigurations[i] < 0)
			continue;

		// Find all the indexes that are required by the selected configuration
		for (int k = 0; k < problemInstance.nIndexes; k++) 
		{
			// If index k belongs to configuration i and has not yet been built
			if (problemInstance.configIndexesMatrix[selectedConfigurations[i]][k] == 1 && indexesToBuild[k] == 0)
			{
				indexesToBuild[k] = 1;										// Mark it as 'used'
				memory += problemInstance.indexesMemoryOccupation[k];		// and add its memory cost to the result
			}
		}
	}

	return memory;
}


long Solution::getObjFunctionValue() const
{
	return objFunctionValue;
}

long Solution::getFitnessValue() const
{
	return fitnessValue;
}

int Solution::getMemoryCost() const
{
	return memory;
}


void Solution::writeToFile(const std::string& fileName) const
{
	std::vector<std::vector<short int>> configsServingQueries;

	// Generate solution matrix on-the-fly before printing it
	for (int i = 0; i < problemInstance.nQueries; i++)
	{
		configsServingQueries.emplace_back(std::vector<short int>(problemInstance.nConfigs, 0));

		// Set value of the proper cell of the column
		if (selectedConfigurations[i] >= 0)
			configsServingQueries[i][selectedConfigurations[i]] = 1;
	}

	FILE* fl;
	fopen_s(&fl, fileName.c_str(), "w");
	if (fl == NULL)
	{
		throw exception(("Error: unable to open file '" + fileName + "'").c_str());
	}
	else
	{	// Print the solution matrix on the output file
		for (int i = 0; i < problemInstance.nConfigs; i++) 
		{
			for (int j = 0; j < problemInstance.nQueries; j++) {
				fprintf_s(fl, "%d ", configsServingQueries[j][i]);
			}
			fprintf_s(fl, "\n");
		}
		fclose(fl);
	}
}
