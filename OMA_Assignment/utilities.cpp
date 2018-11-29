#include "utilities.hpp"


Params parseCommandLine(int argc, char *argv[])
{	// working parsing_CommandLine

	Params execParams = Params();

	if (argc != 2 && argc != 4)
	{
		fprintf(stderr, "Wrong command line parameters usage, expected:"
			"\n$ODBDPsolver_OMAAL_group04.exe <instancefilename> -t <timelimit>\n");
		execParams.parsingError = true;
	}
	else
	{
		for (int i = 1; i < argc; ++i)
		{
			// Parsing the -t <timelimit> parameter, which takes up 2 consecutive args
			if (strcmp(argv[i], "-t") == 0 && i < argc-1)		
			{
				execParams.timeLimit = (unsigned) atoi(argv[i + 1]);
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
				fprintf(stderr, "Unknown usage of command line parameters, expected:"
					"\n$ODBDPsolver_OMAAL_group04.exe <instancefilename> -t <timelimit>\n");
				execParams.parsingError = true;
			}
		}
	}

	return execParams;
}


Instance readInputFile(std::string fileName)
{	// working reading_file

	Instance instance = Instance();
	FILE *fl;

	fl = fopen(fileName.c_str(), "r");
	if (fl == NULL) 
	{
		fprintf(stderr, "Encountered an error when attempting to open and read file '%s'\n", fileName.c_str());
		return Instance();
	}

	int l = fscanf(fl, "%*s%u",&instance.nQueries);
	l += fscanf(fl, "%*s%u",&instance.nIndexes);
	l += fscanf(fl, "%*s%u",&instance.nConfigs);
	l += fscanf(fl, "%*s%u",&instance.M);
	if (l != 4) {
		fprintf(stderr, "Error in the file format\n");
		return Instance();
	}
	
	fscanf(fl, "%*s");	// eliminate an extra row

	// reading the CONFIGURATION_INDEX_MATRIX
	instance.configIndexesMatrix = (short int**) malloc(instance.nConfigs * sizeof(short int*));
	for (unsigned int i = 0; i < instance.nConfigs; i++) {
		instance.configIndexesMatrix[i] = (short int*) malloc(instance.nIndexes * sizeof(short int));
		for (unsigned int j = 0; j < instance.nIndexes; j++) {
			fscanf(fl, "%hd", &instance.configIndexesMatrix[i][j]);
		}
	}
	fscanf(fl, "%*s");	// eliminate an extra row

	// alloc and read vector of Fixed_Cost for each index
	instance.indexesFixedCost = (unsigned int*) malloc (instance.nIndexes * sizeof(unsigned int));
	for (unsigned int i = 0; i < instance.nIndexes; i++) {
		fscanf(fl, "%u", &instance.indexesFixedCost[i]);
	}

	fscanf(fl, "%*s"); //  eliminate an extra row

	// alloc and read vector of Memory needed from every index
	instance.indexesMemoryOccupation = (unsigned int*)malloc(instance.nIndexes * sizeof(unsigned int));
	for (unsigned int i = 0; i < instance.nIndexes; i++) {
		fscanf(fl, "%u", &instance.indexesMemoryOccupation[i]);
	}

	fscanf(fl, "%*s");	//  eliminate an extra row

	// alloc and read the CONFIGURATION_QUERIES_GAIN
	instance.configQueriesGain = (unsigned int **)malloc(instance.nConfigs * sizeof(unsigned int*));
	for (unsigned int i = 0; i < instance.nConfigs; i++) {
		instance.configQueriesGain[i] = (unsigned int*)malloc(instance.nQueries * sizeof(unsigned int));
		for (unsigned int j = 0; j < instance.nQueries; j++)
			fscanf(fl, "%u", &instance.configQueriesGain[i][j]);
	}

	fclose(fl);

	return instance;
}


//////////////////////////////////////////////
//
//	Solution class implementation
//
//////////////////////////////////////////////


bool Solution::isFeasible()
{
	bool FF;
	Solution s; // received as parameter
	short int ***matrX_p = &s.configsServingQueries;		//	|C|x|Q| bool 

	Instance e = readInputFile(filename);
	short int ***matrE_p = &e.configIndexesMatrix;			//	|C|x|I|	bool
	unsigned int **vectF_p = &e.indexesFixedCost;			//	|I|		integer
	unsigned int **vectM_p = &e.indexesMemoryOccupation;	//	|I|		integer
	unsigned int ***matrG_p = &e.configQueriesGain;			//	|C|x|Q| integer

	//	 allocate vector for check 2nd constraint
	unsigned int *check4Query = calloc(e.nQueries, sizeof(unsigned int)); 

	unsigned int *b;					
	unsigned int i, j, k, mem;
	unsigned int M = e.M;
	
	// allocate b vector
	b = calloc(e.nIndexes,sizeof(unsigned int));
	
	//	iterate x matrix 

	for (i = 0; i < s.nC; i++) {
		for (j = 0; j < s.nQ; j++) {

			if (*matrX_p[i][j] == 1) {			// configuration i serve query j

						///		CHECK 2ND CONSTRAINT
				if (check4Query[j] = 1) {
					fprintf(stderr, "Too many configuration for query %u", j);
					return FF = false;
				}					
				else check4Query[j] += 1;
						///		MEMORY CHECK
				for (k = 0; k < e.nIndexes; k++) {
					if (*matrE_p[i][k] == 1) {
						// index k is served by configuration i so it has to be builded
						b[k] = 1;
						// search for memory usage
						mem += *vectM_p[k];
						if (mem > M) {
							fprintf(stderr, "Configuration %u that uses index %u for quey %u exceeds memory limit \n", i, k, j);
							return FF = false;
						}

					}
					//	else b[k] = 0;
				}

			}

		}
	}
	
	 



	// PLEASE NOTE: the third constraint in our model is actually a way to build the 'b' vector
	// which is then used in the objective function evaluation. Therefore, it's not something to check
	// in order to determine the feasibility of the solution but you need to set the proper 0/1 values
	// inside this function
	return false;
}

unsigned long int Solution::evaluateObjectiveFunction()
{
	Instance e = readInputFile(filename);
	unsigned int **vectF_p = &e.indexesFixedCost;			//	|I|		integer
	unsigned int ***matrG_p = &e.configQueriesGain;			//	|C|x|Q| integer

	unsigned int all_gains = 0;
	unsigned int time_spent = 0;	

	// gets gain given a solution
	for (int i = 0; i < e.nConfigs; i++) {
		for (int j = 0; j < e.nQueries; j++) {
			all_gains += (configsServingQueries[i][j])*(*matrG_p[i][j]);
		}
	}

	// gets time spent given a solution
	for (int i = 0; i < e.nIndexes; i++){
		time_spent += indexesToBuild[i]*(*vectF_p[i]);
	}

	// updates objective function value
	objFunctionValue = all_gains - time_spent;

	return objFunctionValue;

	// PLEASE NOTE: after finishing the evaluation, remember to save
	// the result inside objFunctionValue (in the solution class) before returning it

}

void Solution::writeToFile(std::string fileName)
{

		using namespace std;
		ofstream myfile(fileName);
		if (myfile.is_open())
		{
			for ()
			{
				myfile << Solution.C + " ";  //need solution to understand how to print, which variables, data and functions it is possible to use
					cout << Solution.C;
				myfile << Solution.Q;
					cout << Solution.Q;
				myfile << Solution.ecq;
					cout << Solution.ecq;
			}
			myfile.close();
		}
		else cout << "Error: unable to open file";
}
