# mySAT
Created by: Jaime Sanchez

Course: ECE 51216 Digital Logic Design and Automation

Date: May 7th, 2026

mySAT is an implentation of the DPLL algorithm with a two-watched-literal and DLIS branching heuristic.


# How-to-compile (`mySAT.cpp`)
There is no Makefile or CMakefile to complie the mySAT solver. Rather use the following command to compile the code:
```
g++ mySAT.cpp parser.cpp -o mySAT
```

If you want to compile without the DLIS branching heuristic, you can set the following in `mySAT.cpp`:
`#define DLSI_INHIBITED false` to `#define DLSI_INHIBITED true`


# How-to-run (`mySAT.cpp`)

```
./mySAT <path_to_cnf_file>.cnf
```

example:
```
./mySAT simple_benchmarks/mized_sizes.cnf 
```

or use any of the helpful shell scripts:
- `simple_test_runner.sh` - runs 6 claude generated CNF benchmakrs to test the barebones SAT solver.
- `test_runner.sh` - runs multitudes of CNF benchmarks, uses the Uniform Random-3-SAT problem set from https://www.cs.ubc.ca/~hoos/SATLIB/benchm.html


# Expected Output
Option 1 (SAT):
```
RESULT:SAT
ASSIGNMENT:1=0 2=1 3=0 4=1
```

OR

Option 2 (UNSAT):
```
RESULT:UNSAT
```


# Overview Directory structure

- `mySAT.cpp` - solver: data structures, BCP, DLIS, DPLL, main.
- `parser.cpp` - CNF parser to read .cnf file exntsions (DIMACS format)
- `parser.h` - header for parser.
- `simple_test_runner.sh` - helper shell script for running simple bechmarks.
- `test_runner.sh` - helper shell script for running a set of bechmarks in the '/benchmarks' directory.


# Indepth look at the directory structure

## `mySAT.cpp` 
Data structures:
- `Assignment` - enum: `UNASSIGNED`, `ASSIGNED_TRUE`, `ASSIGNED_FALSE`.
- `Literal_Assignments`- struct - current assignment plus a trail for backtracking.
- `Clause` - struct - represents a single clause in the formula.
- `CNF_Formula` - struct - a formula structure that holds all the clauses in the formula.
- `Current_Variable_Node` - struct - represents vairable and polarity. 
- `WatchPositions` - struct - holds litereal watch list index per clause.
- `All_Watched_Literals` - struct - holds literal watch lists and the unit-propagation queue.
- `DLIS_Counter` - struct - positive/negative occurrence counts used by DLIS.

Functions:
- `initialize_cnf_formula` - function - copies parser output into the internal `CNF_Formula` structure.
- `get_watch_list` - function - returns the watch list for a signed literal (positive or negative) (const options).
- `initialize_watched_literals` - function - sets up the two watches per non-unit clause and queues unit clauses for propagation. Will return false on a trivial empty-clause UNSAT.
- `BCP` - function - Boolean Constraint Propagation with two watched literals that returns false on conflict.
- `basic_brancher` - function - simple/naive brancing husritics for testing that returns the lowest-index unassigned variable.
- `DLIS` - function - hols the DLIS heuristic. Returns the literal that appears most often in an unsatisfied clauses.
- `backtrack` - function - pops the trail down, restoring the assignment state.
- `DPLL` - function - main level of the DPLL algorithm. Runs BCP, picks a branching literal, and does recursion.
- `print_assigments` - function - prints the expcted output per the project output format.
- `main` - function - main function to perform DPLL and print output.

## `parser.cpp`
- `parse_dimacs` - implementations of the DIMACS parser with a STREAM or File Path option call.

## `parser.h`
- `parse_dimacs` - declartion of a function that reads a .cnf file and populates the formula data structure.
- `Formula` - struct - holds the number of variables, clauses, and the clauses themselves.
