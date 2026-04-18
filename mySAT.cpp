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
#include <deque>
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
/**
 * Function that perfoms unit clause elimination as a preprocessing step for the DPLL algorithm. 
 * (a.k.a.) If there is a clause with only one literal, then that literal must be assigned a value that satisfies the clause.
 */
int unit_clause(const CNF_Formula& formula) {
    // Placeholder for the preprocessing step of the DPLL algorithm
    // This function should apply the pure literal rule.
    //std::vector<int> literal_assginments;

    /* Count the occurrences of each literal in the formula and assigned the count in a map*/
    
    return 0; // Placeholder return value
}


struct Current_Variable_Node
{
    int variable; /* the variable that is being assigned a value in the current node of the search tree */
    int polarity; /* the polarity of the variable assignment in the current node of the search tree. 1 for true, -1 for false */
};


// struct Watched_Literals_Node
// {
//     int clause_index; /* the index of the clause that is being watched in the current node of the search tree */
//     int watched_literal_1; /* the first watched literal for the clause being watched in the current node of the search tree */
//     int index_of_watched_literal_1; /* the index of the first watched literal in the clause being watched in the current node of the search tree */
//     int watched_literal_2; /* the second watched literal for the clause being watched in the current node of the search tree */
//     int index_of_watched_literal_2; /* the index of the second watched literal in the clause being watched in the current node of the search tree */

//     bool is_clause_satisfied; /* boolean that represents whether the clause being watched is satisfied or not in the current node of the search tree, initialized to false */
// };



struct WatchPositions {
    int first;
    int second;
};

struct All_Watched_Literals {
    // Both are sized (num_vars + 1); index 0 is unused so variable v maps directly to index v.
    // Each index represents a watched literal. The value at each index is a vector of clause thare are currently watching that literal.
    std::vector<std::vector<int>> watches_pos;
    std::vector<std::vector<int>> watches_neg;

    // For each clause, which two literal positions within that clause are watched.
    // For unit clauses, second == -1 is used as a sentinel.
    // Takes into account the 0-based indexing of the literals within the clause.
    std::vector<WatchPositions> clause_watch_positions;

    // Literals forced true/false by unit clauses, awaiting propagation.
    // Stored as signed literals (positive means assign true, negative means assign false).
    std::deque<int> unit_propagation_queue;
};

/**
 * Helper that returns a reference to the watch list for a given signed literal.
 * Positive literals go to watches_pos; negative literals go to watches_neg.
 * For example, if the literal is 2, then the function will return a reference to calueses are on watches_pos[2].
 */
std::vector<int>& get_watch_list(All_Watched_Literals& awl, int literal) {
    if (literal > 0) {
        return awl.watches_pos[literal];
    } else {
        return awl.watches_neg[std::abs(literal)]; //negative literal, so we take the absolute value to index into watches_neg
    }
}

/**
 * Const overload for read-only access during propagation.
 */
const std::vector<int>& get_watch_list(const All_Watched_Literals& awl, int literal) {
    return (literal > 0) ? awl.watches_pos[literal]
                         : awl.watches_neg[std::abs(literal)];
}

/**
 * Function that initializes the watched literals for each clause in the formula.
 * @param formula: the CNF formula for which to initialize watched literals
 * @param awl: the structure to store all watched literals
 * @return false if the formula contains an empty clause (trivially UNSAT), true otherwise
 */
bool initialize_watched_literals(const CNF_Formula& formula, All_Watched_Literals& awl) {
    // Size (num_vars + 1) so variable v maps directly to index v (index 0 unused).
    awl.watches_pos.assign(formula.num_vars + 1, {});
    awl.watches_neg.assign(formula.num_vars + 1, {});
    awl.clause_watch_positions.reserve(formula.clauses.size());

    for (size_t i = 0; i < formula.clauses.size(); ++i) {
        const auto& lits = formula.clauses[i].literals;

        if (lits.empty()) return false;   // trivial UNSAT

        if (lits.size() == 1) {
            // Unit clause: no watches needed for this specific case, queue the literal for propagation.
            awl.unit_propagation_queue.push_back(lits[0]);
            awl.clause_watch_positions.push_back({0, -1});  // sentinel: unit clause
            continue;
        }

        // Normal case: watch the first two literal positions in the clause.
        awl.clause_watch_positions.push_back({0, 1});
        get_watch_list(awl, lits[0]).push_back(static_cast<int>(i));
        get_watch_list(awl, lits[1]).push_back(static_cast<int>(i));
    }
    return true;
}


/**
 * Function that perfoms boolean constraint propagation (BCP) with the two watched literals heuristic. 
 * 
 */
bool bcp(CNF_Formula& formula) {
    
    for(int i = 0; i < formula.clauses.size(); i++)
    {
        
    }
    


    return false; // Placeholder return value
}


bool dpll(CNF_Formula& formula) {
    // Placeholder for the DPLL algorithm implementation
    // This function should return true if the formula is satisfiable, and false otherwise.
    std::cout << "Doing SAT stuff\n";
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

    //==============Verify Arguments and the Benchmark File================
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

    //==============Initiailize data structures for DPLL================

    /* create clauses, redundant but used to seperate AI generated code to my own driven development/design*/
    CNF_Formula my_Formula= initialize_cnf_formula(parsed_cnf_formula);

    /* initialize watched literals data structure */
    All_Watched_Literals watched_literals;
    if(!initialize_watched_literals(my_Formula, watched_literals))
    {
        std::cout << "ERROR: failed to initialize watched literals\n";
        std::cout << "RESULT:UNSAT\n";
        return 1;
    }

    //==============Run SAT Solver algorithm================
    /* run dpll*/
    bool is_satisfiable = dpll(my_Formula);



    //==============Print Output================
    if (is_satisfiable) {
        std::cout << "RESULT:SAT\n";
        //TODO print litral assignments
    } else {
        std::cout << "RESULT:UNSAT\n";
    }

    return 1;
}
