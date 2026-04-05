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
//#include <map>

/**
 * Structure that represents a clause in the CNF formula. It contains a 
 * vector of interegers that represent the literals in the clause. 
 */
struct Clause 
{
    std::vector<int> literals;  /* list of lits in a single clause*/
    bool is_satisfied; /* boolean that represents whether the clause is satisfied or not, initialized to false */

};

/**
 * Structure that represents the whole given CNF formula. 
 * 
 * It contains the number of variables, the number of clauses, and a 
 * vector of Clause objects that represent the clauses in the formula.
 */
struct CNF_Formula 
{
    int num_vars;              /* number of variables in the CNF formula */
    int num_clauses;           /* number of clauses in the CNF formula */
    std::vector<Clause> clauses; /* vector of clauses in the CNF formula */

    std::vector<int> positive_literal_assignments; /* vector of positive literal assignments. 1 for true, -1 for false, 0 for unassigned */
    std::vector<int> negative_literal_assignments; /* vector of negative literal assignments. 1 for true, -1 for false, 0 for unassigned */

    std::vector<int> final_literal_assignments; /* vector of final literal assignments. 1 for true, -1 for false, 0 for unassigned */
};

/**
 * Function that copies over the information from the parser file output in to a CNF_Formula structure within scope of this projct.
 * @param parsed_cnf_formula: the formula structure output from the parser file.
 * @return CNF_Formula: the formula structure used within this project.
 */
CNF_Formula initialize_cnf_formula(const Formula& parsed_cnf_formula) {
    CNF_Formula this_formula;
    this_formula.num_vars = parsed_cnf_formula.num_vars;
    this_formula.num_clauses = parsed_cnf_formula.num_clauses;

    this_formula.positive_literal_assignments.resize(this_formula.num_vars + 1, 0); // Initialize positive literal assignments to 0 (unassigned)
    this_formula.negative_literal_assignments.resize(this_formula.num_vars + 1, 0); // Initialize negative literal assignments to 0 (unassigned)

    /* Copy clauses from the parsed formula to the CNF_Formula structure, assigning initial values to the +/- literal assignments */
    for (int i = 0; i < parsed_cnf_formula.clauses.size(); i++)
    {
        Clause clause;
        clause.literals = parsed_cnf_formula.clauses[i];  // Copy literals from the parsed clause
        clause.is_satisfied = false; // Initialize the clause as unsatisfied
        for(int j=0; j<clause.literals.size(); j++)
        {
            if(clause.literals[j] > 0)
            {
                this_formula.positive_literal_assignments[clause.literals[j]] = 1; // Positive literal 
            }
            else if(clause.literals[j] < 0)
            {
                this_formula.negative_literal_assignments[abs(clause.literals[j])] = -1; // Negative literal
            }
            else
            {
                this_formula.negative_literal_assignments[abs(clause.literals[j])] = 0; // Unassigned literal
            }
        }

        this_formula.clauses.push_back(clause);
    }
    return this_formula;
}
// /**
//  * Function that simplies the formula by first looking at any literal that appears in only one polarity.
//  * Use to preform the pure literal rule before the main DPLL recursion.
//  */
// int preprocess(const CNF_Formula& formula) {
//     // Placeholder for the preprocessing step of the DPLL algorithm
//     // This function should apply the pure literal rule.
//     //std::vector<int> literal_assginments;

//     /* Count the occurrences of each literal in the formula and assigned the count in a map*/
    
//     return 0; // Placeholder return value
// }


struct Current_Variable_Node
{
    int variable; /* the variable that is being assigned a value in the current node of the search tree */
    int polarity; /* the polarity of the variable assignment in the current node of the search tree. 1 for true, -1 for false */
};

/**
 * Function that assigns a clause if it is satisfied or not based on the current variable assignments from the search tree.
 */
std::vector<int> mark_clauses_assignment(CNF_Formula& formula, const Current_Variable_Node& current_node) {


    for(int i=0; i<formula.clauses.size(); i++)
    {
        /* if a clause is not satified, look to see if the current choosen node polarity and variable assgiment match to satisfy the caluse*/
        if(!formula.clauses[i].is_satisfied)
        {
            for(int j=0; j<formula.clauses[i].literals.size(); j++)
            {
                if(formula.clauses[i].literals[j] == current_node.variable * current_node.polarity) 
                {
                    formula.clauses[i].is_satisfied = true; // Mark the clause as satisfied
                    break; // No need to check other literals in this clause
                }
            }
        }
    }

    return std::vector<int>(); // Placeholder return value
}









bool bcp(CNF_Formula& formula) {
    
    for(int i = 0; i < formula.clauses.size(); i++)
    {
        /* if a clause is not satified, look to see if there is a single unassigned literal */
        if(!formula.clauses[i].is_satisfied)
        {
            for(int j = 0; j < formula.clauses[i].literals.size(); j++)
            {
                if(formula.clauses[i].literals[j] == 0) 
                {
                    formula.clauses[i].is_satisfied = true; // Mark the clause as satisfied
                    break; // No need to check other literals in this clause
                }
            }
            
        }
    }
    
    return false; // Placeholder return value
}


bool dpll(CNF_Formula& formula) {
    // Placeholder for the DPLL algorithm implementation
    // This function should return true if the formula is satisfiable, and false otherwise.

    /* base case, look for for unit clauses*/
    if(bcp(formula)){
        return true; // all clauses are satisfied
    }



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

    /* run dpll*/
    bool is_satisfiable = dpll(my_Formula);




    if (is_satisfiable) {
        std::cout << "RESULT:SAT\n";
        //TODO print litral assignments
    } else {
        std::cout << "RESULT:UNSAT\n";
    }

    return 1;
}
