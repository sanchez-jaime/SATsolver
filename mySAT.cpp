/**
 * mySAT.cpp
 * 
 * 
 * ECE 51216
 * Created by Jaime Sanchez
 */

#include "parser.h"

//#include <algorithm>
//#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

/**
 * Structure that represents a clause in the CNF formula. It contains a 
 * vector of interegers that represent the literals in the clause. 
 */
struct Clause {
    std::vector<int> literals;  /* list of lits in a single clause*/
};

/**
 * Structure that represents the whole given CNF formula. 
 * 
 * It contains the number of variables, the number of clauses, and a 
 * vector of Clause objects that represent the clauses in the formula.
 */
struct CNF_Formula {
    int num_vars;              /* number of variables in the CNF formula */
    int num_clauses;           /* number of clauses in the CNF formula */
    std::vector<Clause> clauses; /* vector of clauses in the CNF formula */
};

/**
 * Function that cpoies over the information from the parser file output in to a CNF_Forumla structure within scope of this projct.
 * @param parsed_cnf_formula: the formula structure output from the parser file.
 * @return CNF_Formula: the formula structure used within this project.
 */
CNF_Formula initialize_cnf_formula(const Formula& parsed_cnf_formula) {
    CNF_Formula this_formula;
    this_formula.num_vars = parsed_cnf_formula.num_vars;
    this_formula.num_clauses = parsed_cnf_formula.num_clauses;

    /* Copy clauses from the parsed formula to the CNF_Formula structure */
    for (int i = 0; i < parsed_cnf_formula.clauses.size(); i++) {
        Clause clause;
        clause.literals = parsed_cnf_formula.clauses[i];  // Copy literals from the parsed clause
        this_formula.clauses.push_back(clause);
    }
    return this_formula;
}

bool dpll(const CNF_Formula& formula) {
    // Placeholder for the DPLL algorithm implementation
    // This function should return true if the formula is satisfiable, and false otherwise.
    return false; // Placeholder return value
}












/**
 * main function
 * @params argc: number of command line arguments
 * @params argv: array of command line arguments
 */

int main(int argc, char* argv[]) {

    /* verify number of args given*/
    if (argc != 2)
    {
        std::cout << "RESULT:UNSAT\n";
        return 1;
    }

    std::string benchmark_file;
    benchmark_file = argv[1];

    /* check if a valid file was given*/
    if(benchmark_file.empty())
    {
        std::cout << "RESULT:UNSAT\n";
        return 1;
    }

    /* parse benchmark file*/
    Formula parsed_cnf_formula;
    if (!parse_dimacs(benchmark_file, parsed_cnf_formula)) 
    {
        /* failed to parse */
        std::cout << "RESULT:UNSAT\n";
        return 1;
    }

    /* create clauses*/
    CNF_Formula my_Formula= initialize_cnf_formula(parsed_cnf_formula);

    bool is_satisfiable = dpll(my_Formula);

    if (is_satisfiable) {
        std::cout << "RESULT:SAT\n";
    } else {
        std::cout << "RESULT:UNSAT\n";
    }

    return 1;
}
