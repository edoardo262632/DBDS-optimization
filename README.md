# Heuristic solver for the ODBDP
This repository contains the code of a genetic algorithm, developed as an assignment for the Optimization Methods and Algorithms course at PoliTO in 2019, which aims at heuristically solving the ODBDP.

The algorithm's code was later rewritten in pure C++ to improve its performance and get rid of many C-like elements that were left in the previous implementation; all of the original source files along with our results, reports and presentation can still be found [here](https://github.com/edoardo262632/DBDS-optimization/releases/download/v1.0/ODBDPproject_OMAAL_group04.zip), a brief summary of that information is contained in the following paragraphs.

# Index
1. [The problem](#The-problem)
2. [Algorithm design](#Algorithm-design)
3. [Results and considerations](#Results-and-considerations)

# The problem
The **Optimal DataBase Design Problem** (*ODBDP*) is the problem of translating the logical data model into technical specifications able to physically create a database (DB) on a machine; mostly defining an appropriate set of access structures which offer a good compromise between memory occupation and time required for data retrieval and maintenance. A particularly important phase of the design consists of choosing the indexes to be created in order to globally minimize the response time for a given workload (queries on the DB).

A **problem instance** consists in:
- the set of Q *queries*
- the set of I possible *indexes* that can be built to serve the queries, each comes with a fixed time cost (f > 0) and a memory cost (m > 0) for its creation and maintenance
- the set of C index *configurations*, which serve a subset of the queries (providing a variable execution time gain, g > 0)
- the maximum memory capacity M > 0

The ODBDP aims at selecting the indexes to be built for the DB (consistently with the activated configurations) ensuring that: 1) at most one index configuration is used for each query and 2) the total memory usage does not exceed M. The objective is to maximize the net gain in terms of total time (i.e., also considering creation and maintenance times) to execute all the queries on the DB.

# Algorithm design
After defining a *Linear Programming (LP)* model for the ODBDP, we worked on designing a heuristic algorithm that could solve the problem efficiently in a time-constrained scenario: we chose to implement a genetic algorithm for this purpose.

**Note:** our solution consists in a vector of |Q| integer values, that indicate which configuration was chosen to serve each query (or -1 if the query is left unserved); this representation automatically satisfies the first (uniqueness) constraint of the problem since each query can only be served by a single configuration.

Here's a brief outline of our algorithm's structure:
- ***Initialization:*** 2 greedy initializers are available (each with lots of randomness), the choice between them is randomly done at start time
- ***Parent selection:*** completely random across the current population
- ***Crossover:*** N-point crossover is employed (with N randomised between 2 and 6 at each generation), swapping odd chromosomes of the solution vectors
- ***Mutation:*** each gene of an individual has a 1/|Q| probability of mutating, if the mutation happens it can either cause to choose another configuration that servers that query (90% probability) or to reset the query to its default configuration (10% probability)
- ***Fitness function:*** it is a modified version of the objective function, in which infeasible solutions are penalized by their surplus memory usage
- ***Population replacement:*** the solutions (all parents and offsprings) are ranked by their fitness value and the current population is replaced by the POPULATION_SIZE best solutions
- ***Local Search:*** a rudimental first-improvement LS is run on the population to refine the individuals if the algorithm has been stuck for a few generations
- ***Multi-start:*** in case the algorithm gets stuck in a local optimum for many generations, it returns to the initialization step and restarts with a new randomised population, to explore another part of the solution space

![Algorithm structure image](https://imgur.com/JiFDbpx.jpg)

In addition to everything we just described, our algorithm can run N threads in parallel, each working with an independent population, a random number generator and its own set of variables, while the current best solution is shared across all threads. This allows us to multiply by a factor of N the effectiveness of our multi-start technique, since a higher number of starting points and populations are considered.

# Results and considerations
To evaluate the algorithm's performance we ran benchmarks on a set of 20 provided ODBDP instances, comparing our results against the optimal solutions or, in those cases when those were unknown, the best solutions available up to that point.
All of our tests were run with an execution time of 3 minutes and 4 threads running in parallel.

![Benchmarks image](https://imgur.com/a9kkAEO.jpg)

Our algorithm was able to find very good solutions for a few of the instances, while in other cases it fell short reaching results 10 to 20% worse compared to the example solutions.

The worst results (> 10% gap) were obtained when the objective function and memory used by a solution have different order of magnitude, this exposed some issues in our fitness function formulation (lack of a scale factor for the penalization inflicted to infeasible solutions), that may have hindered performance; a better fitness function would probably be the first thing we'd implement if we had to improve this algorithm.

As an additional consideration, the convergence time (to local optima, that is) of this algorithm is extremely fast and therefore it is very good if run with a very strict time limit (< 60s) but since the assignment didn't have such strict time execution constraints it would have probably better to embed more complexity (e.g. better selection, deeper LS...) into the algorithm to obtain better solutions.
