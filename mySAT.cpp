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


struct Current_Variable_Node
{
    int variable; /* the variable that is being assigned a value in the current node of the search tree */
    int polarity; /* the polarity of the variable assignment in the current node of the search tree. 1 for true, -1 for false */
};

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
    if (literal > 0) 
    {
        return awl.watches_pos[literal];
    } 
    else
    {
        return awl.watches_neg[std::abs(literal)]; //negative literal, so we take the absolute value to index into watches_neg
    }
}

/**
 * Helper function that only reads the watch list for a given signed literal. Positive literals go to watches_pos; negative literals go to watches_neg.
 */
const std::vector<int>& get_watch_list(const All_Watched_Literals& awl, int literal) {
    if (literal > 0) 
    {
        return awl.watches_pos[literal];
    } 
    else
    {
        return awl.watches_neg[std::abs(literal)]; //negative literal, so we take the absolute value to index into watches_neg
    }
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
    awl.clause_watch_positions.reserve(formula.clauses.size()); //reserve space for the watch positions of each clause to avoid unnecessary reallocations

    for (size_t i = 0; i < formula.clauses.size(); ++i) 
    {
        const std::vector<int>& lits = formula.clauses[i].literals; // const to avoid oopsies

        if (lits.empty()) return false;   // trivial UNSAT

        if (lits.size() == 1) 
        {
            // Unit clause: no watches needed for this specific case, queue the literal for propagation.
            awl.unit_propagation_queue.push_back(lits[0]);
            awl.clause_watch_positions.push_back({0, -1});  // unit clause
            continue;
        }

        // Normal case: watch the first two literal positions in the clause.
        awl.clause_watch_positions.push_back({0, 1});
        get_watch_list(awl, lits[0]).push_back(static_cast<int>(i));
        get_watch_list(awl, lits[1]).push_back(static_cast<int>(i));
    }
    return true;
}


enum Assignment : int {
    UNASSIGNED      = 0,
    ASSIGN_TRUE     = 1,
    ASSIGN_FALSE    = -1
};

/**
 * Structure to hold literal assignments during the solving process.
 * Using struct for simplicity, can be refactored to a class if needed in the future
 */
struct Literal_Assignments {
    std::vector<Assignment> tracking_assignments; //vector used to track literal assignments through propagation and backtracking
    std::vector<int> tracking_trail_assignments; //vector for the trail of literals, used for backtracking

    /**
     * Constructor
     * @param num_vars: the number of variables in the CNF formula
     */
    explicit Literal_Assignments(int num_vars) : tracking_assignments(num_vars + 1, UNASSIGNED) {}

    /**
     * Helper function to assign a literal and update the tracking and trail vectors
     * @param literal: the literal to be assigned
     */
    void assign_literal(int literal) {
        if(literal != 0) 
        {
            int var = std::abs(literal);

            // Only assign if the variable is currently unassigned.
            if(tracking_assignments[var] != UNASSIGNED) 
            {
                std::cout << __func__ << "() ERROR: Attempting to assign a literal that is already assigned \n";
            }
            //checking for poloarity in the case to make the potential clause SAT, if x1', then setting x1'=FALSE makes the clause SAT, if x1, then setting x1=TRUE makes the clause SAT
            else if(literal > 0) {
                tracking_assignments[var] = ASSIGN_TRUE; // Assign true to the variable
            } 
            else 
            {
                tracking_assignments[var] = ASSIGN_FALSE; // Assign false to the variable
            }
            tracking_trail_assignments.push_back(literal); // Add the assigned literal to the trail for backtracking purposes
        }
        else{
            std::cout << __func__ << "() ERROR: Attempting to assign an invalid literal (0) \n";
        }  
    }

    /**
     * Helper function to remove the last entry in the trail and unassign the corresponding variable
     */
    void unassign_last_literal() {
        if(tracking_trail_assignments.empty()) 
        {
            std::cout << __func__ << "() ERROR: Attempting to unassign from an empty trail \n";
            return;
        }
        int var = std::abs(tracking_trail_assignments.back()); // Get the last assigned literal from the trail
        tracking_trail_assignments.pop_back(); // Remove the last assigned literal from the trail
        tracking_assignments[var] = UNASSIGNED; // Unassign the variable
    }   

    /**
     * Getter function to return the assignment of the literal, regardless of polarity
     * @param literal: the literal for which to get the assignment
     */
    int get_literal_assignment(int literal) const {
        if(literal == 0) 
        {
            std::cout << __func__ << "() ERROR: Attempting to get assignment for an invalid literal (0) \n";

            return 0; // Not a valid literal
        }
        int var = std::abs(literal);
        int assignment = tracking_assignments[var];   

        // return based on the polarity of the literal. Logic based on assignage in assign_literal() 
        if(literal > 0)
        {
            return assignment;
        }
        else
        {
            return -assignment;
        }
    }

    /**
     * Getter function to return the size of tracking_trail_assignments vector
     */
    size_t get_trail_assignments_size() const {
        return tracking_trail_assignments.size(); // Return the size of the trail, which indicates how many literals have been assigned
    }

    /**
     * Getter function to return the final literal assigment from the tracking assignments vector, taking into account the polarity of the literal
     */
    const std::vector<Assignment>& get_final_literal_assignment() const {
        //TODO update to align with final prinout formart
        return tracking_assignments; // Return the current state of all variable assignments, which can be used to determine the final literal assignments after the solving process is complete
    }
};






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
