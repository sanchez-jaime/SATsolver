/**
 * mySAT.cpp
 * 
 * DPLL SAT Solver with Two Watched Literals and DLIS Branching Heuristic
 * 
 * Created by Jaime Sanchez
 * May 7th, 2026
 * ECE 51216
 */

#include "parser.h"

#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <deque>

#define DLSI_INHIBITED false


/**
 * Enum to represent the assignment state of a variable.
 * 
 */
enum Assignment : int {
    UNASSIGNED      =   0,
    ASSIGNED_TRUE   =   1,
    ASSIGNED_FALSE  =   -1
};


/**
 * Structure to hold literal assignments during the solving process.
 * Using struct for simplicity, can be refactored to a class if needed in the future
 */
struct Literal_Assignments {
    std::vector<Assignment> tracking_assignments;   /* vector used to track literal assignments through propagation and backtracking */
    std::vector<int> tracking_trail_assignments;    /* vector for the trail of literals, used for backtracking */

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

            /* Assign if the variable is currently unassigned. */
            if(tracking_assignments[var] != UNASSIGNED) 
            {
                std::cout << __func__ << "() ERROR: Attempting to assign a literal that is already assigned \n";
            }
            /* checking for poloarity in the case to make the potential clause SAT, if x1', then setting x1'=FALSE makes the clause SAT, 
            if x1, then setting x1=TRUE makes the clause SAT */
            else if(literal > 0) {
                tracking_assignments[var] = ASSIGNED_TRUE;  /* Assign true to the variable */
            } 
            else 
            {
                tracking_assignments[var] = ASSIGNED_FALSE; /* Assign false to the variable */
            }
            tracking_trail_assignments.push_back(literal);  /* Add the assigned literal to the trail for backtracking purposes */
        }
        else{
            std::cout << __func__ << "() ERROR: Attempting to assign an invalid literal (0) \n";
        }  
    }

    /**
     * Helper function to remove the last entry in the trail and unassign the corresponding variable
     * @param none
     * @return void
     */
    void unassign_last_literal() {
        if(tracking_trail_assignments.empty()) 
        {
            std::cout << __func__ << "() ERROR: Attempting to unassign from an empty trail \n";
            return;
        }
        int var = std::abs(tracking_trail_assignments.back());  /* Get the last assigned literal from the trail */
        tracking_trail_assignments.pop_back();                  /* Remove the last assigned literal from the trail */
        tracking_assignments[var] = UNASSIGNED;                 /* Unassign the variable */
    }   

    /**
     * Getter function to return the assignment of the literal, regardless of polarity
     * @param int: the literal for which to get the assignment
     * @return int: the assignment of the literal, taking into account the polarity. 
     */
    int get_literal_assignment(int literal) const {
        if(literal == 0) 
        {
            std::cout << __func__ << "() ERROR: Attempting to get assignment for an invalid literal (0) \n";
            /*not a valid literal*/
            return 0;
        }
        int var = std::abs(literal);
        int assignment = tracking_assignments[var];   

        /* return based on the true polarity of the literal. Logic based on assignage in assign_literal()*/
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
     * @param none
     * @return size_t
     */
    size_t get_trail_assignments_size() const {
        return tracking_trail_assignments.size();   /* Return the size of the trail, which indicates how many literals have been assigned */
    }

    /**
     * Getter function to return the final literal assigment from the tracking assignments vector
     * @param none
     * @return std::vector<Assignment>
     */
    const std::vector<Assignment>& get_final_literal_assignment() const {
        return tracking_assignments;    /* Return the current state of all variable assignments, which can be used to determine the final 
                                        literal assignments after the solving process is complete */
    }
};



/**
 * Structure that represents a clause in the CNF formula. It contains a 
 * Vector of interegers that represent the literals in the clause. 
 * Bollean to represent whether the clause is satisfied or not.
 */
struct Clause 
{
    std::vector<int> literals;  /* list of lits in a single clause */
    bool is_satisfied; 

    /**
     * Helper function that returns a boolean if the literal in a clause makes the caluse satisfied
     * @return bool
     * @param Literal_Assignments: structure for literal assigments
     */
    bool assign_satisfacctions(Literal_Assignments& literal_assigments){
        for(int i = 0; i < literals.size(); i++)
        {
            if(literal_assigments.get_literal_assignment(literals[i]) == ASSIGNED_TRUE)
            {
                return true;
            }
        }
        return false;
    }

};

/**
 * Structure that represents the whole given CNF formula. 
 * It contains the number of variables, the number of clauses, and a 
 * vector of Clause objects that represent the clauses in the formula.
 */
struct CNF_Formula 
{
    int num_vars;              /* number of variables in the CNF formula */
    int num_clauses;           /* number of clauses in the CNF formula */
    std::vector<Clause> clauses; /* vector of clauses in the CNF formula */

    std::vector<int> positive_literal_assignments; /* TBD vector of positive literal assignments. 1 for true, 
                                                    -1 for false, 0 for unassigned */
    std::vector<int> negative_literal_assignments; /* TBD vector of negative literal assignments. 1 for true, 
                                                    -1 for false, 0 for unassigned */
};

/**
 * Function that copies over the information from the parser file output in to a CNF_Formula structure within scope of this projct.
 * @param Formula: the formula structure output from the parser file.
 * @return CNF_Formula: the formula structure used within this project.
 */
CNF_Formula initialize_cnf_formula(const Formula& parsed_cnf_formula) {
    CNF_Formula this_formula;
    this_formula.num_vars = parsed_cnf_formula.num_vars;
    this_formula.num_clauses = parsed_cnf_formula.num_clauses;

    this_formula.positive_literal_assignments.resize(this_formula.num_vars + 1, 0); /* Initialize positive literal assignments to 0 */
    this_formula.negative_literal_assignments.resize(this_formula.num_vars + 1, 0); /* Initialize negative literal assignments to 0 */

    /* Copy clauses from the parsed formula to the CNF_Formula structure, assigning initial values to the +/- literal assignments */
    for (int i = 0; i < parsed_cnf_formula.clauses.size(); i++)
    {
        Clause clause;
        clause.literals = parsed_cnf_formula.clauses[i];  /* Copy literals from the parsed clause */
        clause.is_satisfied = false;    /* Initialize the clause as unsatisfied */
        for(int j=0; j<clause.literals.size(); j++)
        {
            if(clause.literals[j] > 0)
            {
                this_formula.positive_literal_assignments[clause.literals[j]] = 1; /* Positive literal */
            }
            else if(clause.literals[j] < 0)
            {
                this_formula.negative_literal_assignments[abs(clause.literals[j])] = -1; /* Negative literal */
            }
            else
            {
                this_formula.negative_literal_assignments[abs(clause.literals[j])] = 0; /* Unassigned literal */
            }
        }

        this_formula.clauses.push_back(clause);
    }
    return this_formula;
}

/**
 * Structure to represent the current variable assignment in the search tree, used for branching and backtracking.
 * 
 */
struct Current_Variable_Node
{
    int variable; /* the variable that is being assigned a value in the current node of the search tree */
    int polarity; /* the polarity of the variable assignment in the current node of the search tree. 1 for true, -1 for false */
};

/**
 * Structure to represent index positions of the two watched literals within a clause.
 * 
 */
struct WatchPositions {
    int first;
    int second;
};

/**
 * Structure to represent all the watched literals in the formula, including the watch lists for each literal.
 * 
 */
struct All_Watched_Literals {
    std::vector<std::vector<int>> watches_pos; /* Vector of watch lists for positive literals */
    std::vector<std::vector<int>> watches_neg; /* Vector of watch lists for negative literals */

    /* For each clause, which two literal positions within that clause are watched.
     * For unit clauses, second == -1 is used as a sentinel.
     * Takes into account the 0-based indexing of the literals within the clause.
     */
    std::vector<WatchPositions> clause_watch_positions;

    /* Literals forced true/false by unit clauses, awaiting propagation.
     * Stored as signed literals (positive means assign true, negative means assign false).
     */ 
    std::deque<int> unit_propagation_queue;

    /**
     * Helper function to clear the unit propagation queue, used during backtracking to reset the state for the next branch.
     * @param none
     * @return void
     */
    void clear_unit_propagation_queue(){
        unit_propagation_queue.clear();
        return;
    }

};

/**
 * Helper that returns a reference to the watch list for a given signed literal.
 * Positive literals go to watches_pos; negative literals go to watches_neg.
 * For example, if the literal is 2, then the function will return a reference to calueses are on watches_pos[2].
 * @param All_Watched_Literals&: a reference to the structure that holds all the watched literals
 * @param int: the literal for which to get the watch list, can be positive or negative
 * @return std::vector<int>&: a reference to the watch list for the given
 */
std::vector<int>& get_watch_list(All_Watched_Literals& awl, int literal) {
    if (literal > 0) 
    {
        return awl.watches_pos[literal];
    } 
    else
    {
        return awl.watches_neg[std::abs(literal)];  /* negative literal, so we take the absolute value to index into watches_neg */
    }
}

/**
 * Helper function that only reads the watch list for a given signed literal.
 * @param All_Watched_Literals&: a reference to the structure that holds all the watched literals
 * @param int: the literal for which to get the watch list, can be positive or negative
 * @return const std::vector<int>&: a const reference to the watch list
 */
const std::vector<int>& get_watch_list(const All_Watched_Literals& awl, int literal) {
    if (literal > 0) 
    {
        return awl.watches_pos[literal];
    } 
    else
    {
        return awl.watches_neg[std::abs(literal)];  /* negative literal, so we take the absolute value to index into watches_neg */
    }
}

/**
 * Function that initializes the watched literals for each clause in the formula.
 * @param formula: the CNF formula for which to initialize watched literals
 * @param awl: the structure to store all watched literals
 * @return bool: return false if a trivial UNSAT is detected (i.e., an empty clause), otherwise return true.
 */
bool initialize_watched_literals(const CNF_Formula& formula, All_Watched_Literals& awl) {
    /* Size (num_vars + 1) so variable v maps directly to index v (index 0 unused). */
    awl.watches_pos.assign(formula.num_vars + 1, {});
    awl.watches_neg.assign(formula.num_vars + 1, {});
    awl.clause_watch_positions.reserve(formula.clauses.size());     /* reserve space for the watch positions of each clause */

    for (size_t i = 0; i < formula.clauses.size(); ++i) 
    {
        const std::vector<int>& lits = formula.clauses[i].literals; /* const to avoid oopsies */

        if (lits.empty())
        {
            return false;   /* trivial UNSAT */
        }

        if (lits.size() == 1) 
        {
            /* Unit clause: no watches needed for this specific case, queue the literal for propagation. */
            awl.unit_propagation_queue.push_back(lits[0]);
            awl.clause_watch_positions.push_back({0, -1});  /* unit clause */
            continue;
        }

        /* Normal case: watch the first two literal positions in the clause. */
        awl.clause_watch_positions.push_back({0, 1});
        get_watch_list(awl, lits[0]).push_back(static_cast<int>(i));
        get_watch_list(awl, lits[1]).push_back(static_cast<int>(i));
    }
    return true;
}

/**
 * Function that perfoms boolean constraint propagation (BCP) with the two watched literals heuristic. 
 * @param CNF_Formula&: reference to the CNF formula for which to perform BCP
 * @param All_Watched_Literals&: reference to the structure that holds all the watched literals
 * @param Literal_Assignments&: reference to the structure that holds all the literal assignments, which will be updated during BCP
 * @return bool: returns bool FALSE if conflit, otherwise TRUE
 */
bool BCP(CNF_Formula& formula, All_Watched_Literals& awl, Literal_Assignments& literal_assignments){
    
    /* loop to look and apply any queued up literal assignments*/
    while(!awl.unit_propagation_queue.empty()) 
    {
        int front_literal = awl.unit_propagation_queue.front(); /* Get the next literal to propagate */
        awl.unit_propagation_queue.pop_front(); /* Remove it from the queue */

        /* has the front literal been assigned? */
        int front_literal_assignment = literal_assignments.get_literal_assignment(front_literal);
        if(front_literal_assignment != UNASSIGNED)
        {
            if(front_literal_assignment == ASSIGNED_TRUE) 
            {
                /* ignore, this literal has been assgined as true */
                continue;
            }
            if (front_literal_assignment == ASSIGNED_FALSE) 
            {
                /* clear queue for bactracking */
                awl.unit_propagation_queue.clear(); 
                return false;
            }
        }
        
        /*assign queued literal*/
        literal_assignments.assign_literal(front_literal);

        /* Maintain invariants (update watched list to see which literal needs to be moved) */
        int opposite_front_literal = -(front_literal); //opsite literal polarity

        std::vector<int>& watched_list = get_watch_list(awl, opposite_front_literal);
        /* loop through all the clauses in which the opposite assigned variable is assigned to to replace it in the watched list */
        for(int i = 0; watched_list.size() > i; i++)
        {
            int watched_clause_number = watched_list[i];
            int right_position = 0;
            int second_watched_position = 0;

            std::vector<int>& literals_in_clause = formula.clauses[watched_clause_number].literals; /* get all the litereals in a 
                                                                                                     * clause that contained the opposite 
                                                                                                     * queued up literal */

            WatchPositions& first_second_positions = awl.clause_watch_positions[watched_clause_number]; /* reference for updating stuff */

            if(literals_in_clause[first_second_positions.first] == opposite_front_literal)
            {
                /* target index is the literal */
                right_position = first_second_positions.first;
                second_watched_position = first_second_positions.second;
            }
            else if (literals_in_clause[first_second_positions.second] == opposite_front_literal)
            {
                /* target index is the literal */
                right_position = first_second_positions.second;
                second_watched_position = first_second_positions.first;
            }

            int litereal_of_second_watched_position = literals_in_clause[second_watched_position];  /* get the 'other' watched literal */

            /* target literal 'opposite_front_literal', second literal in watched list 'litereal_of_second_watched_position' */
            if(literal_assignments.get_literal_assignment(litereal_of_second_watched_position) == ASSIGNED_TRUE)
            {
                /* if the other watched literal already satifuies the clause, igore the current literal target*/
                continue;   /* move on */
            }
            bool updated_watch_list = false;

            /* update watch list, loop through all the literals in a clause*/
            for(int j = 0; j < literals_in_clause.size(); j++)
            {
                /* find next suitable literal index to watch */
                if(first_second_positions.first == j || first_second_positions.second == j)
                {
                    continue; /* skip */
                }
                int new_literal_replacement_assigment = literal_assignments.get_literal_assignment(literals_in_clause[j]);

                if(new_literal_replacement_assigment == ASSIGNED_FALSE)
                {
                    /* Skip since new replacement already assgined*/
                    continue; 
                }
                
                /* update pointers and flip */
                if(right_position == first_second_positions.first)
                {
                    first_second_positions.first = j; 
                    
                }
                else if (right_position == first_second_positions.second)
                {
                    first_second_positions.second = j; 
                }
                
                /* leverage vector's pop_back() and back(). Order doesnt matter as long as get go through all of them  */
                watched_list[i] = watched_list.back();
                watched_list.pop_back();
                updated_watch_list = true; 

                get_watch_list(awl, literals_in_clause[j]).push_back(watched_clause_number);
                break;
            }
            if(updated_watch_list)
            {
                i--;    /* decrement i since we just swapped the current index with the last index and popped the last index, so we need to 
                         * check the current index again since it now contains a new clause number after the swap */
                continue; /* move on to the next clause in the watched list since we successfully updated the watch list for this clause */
            }
            else{
                int second_watched_literal_assignment = literal_assignments.get_literal_assignment(litereal_of_second_watched_position);
                if(second_watched_literal_assignment == UNASSIGNED)
                {
                    /* forced decision, new unit clause and add to queue */
                    awl.unit_propagation_queue.push_back(litereal_of_second_watched_position);
                }
                /* if there are no more literals to watch, and the other watched literal is assigned false, then we have a conflict, return false*/
                else if(second_watched_literal_assignment == ASSIGNED_FALSE)
                {
                    /* clear queue for bactracking */
                    awl.unit_propagation_queue.clear(); 
                    return false;
                }
            }
        }

    }
    /* BCP completed without conflict */
    return true;
}


/**
 * Function that performs basic DPLL branching without Branching DLIS Heuristics
 * @param CNF_Formula&: reference to the CNF formula for which to perform basic branching
 * @param Literal_Assignments&: reference to the structure that holds all the literal assignments
 * @return int: new literal to branch on
 */
int basic_brancher(CNF_Formula& formula, Literal_Assignments& literal_assignments){
    for(int i = 1; i < formula.num_vars + 1; i++){
        /*Pick new vairable so long as it has not beed assigned yet*/
        int temp_new_literal = literal_assignments.tracking_assignments[i];
        if ((temp_new_literal == ASSIGNED_TRUE) || (temp_new_literal == ASSIGNED_FALSE)){
            continue; //skip 
        }
        else{
            return i; //found one
        }
        return 0; //needs to be zero, as that is the only unused number
    }
    return 0;
}

struct DLIS_Counter {
    std::vector<int> positive_count; //vector to count the number of times each unassigned positive literal appears in an unsatisfied clause
    std::vector<int> negative_count; //vector to count the number of times each unassigned negative literal appears in an unsatisfied clause

     /**
     * Constructor
     * @param num_vars: the number of variables in the CNF formula
     */
    explicit DLIS_Counter(int num_vars) : positive_count(num_vars + 1, 0), negative_count(num_vars + 1, 0) {} 
};

/**
 * Function that implements the DLIS branching heuristic to pick the next literal to branch on.
 * @param formula: the CNF formula for which to pick the next branching literal
 * @param literal_assignments: the current literal assignments
 * @return the next literal to branch on, based on the DLIS heuristic
 */
int DLIS(CNF_Formula& formula, Literal_Assignments& literal_assignments){
    /* Scan all the unsatisfied clauses
     * record the number of times each unassigned literal appears in an unsatisfied clause
     * keep track of the literal with different polarity that appears the most, and return that literal as the new branching literal  
    */
    int new_literal = 0;
    int best_counter = 0;
    DLIS_Counter dlis_counter(formula.num_vars);

    for(int i = 0; i < formula.clauses.size(); i++)
    {
        Clause& clause = formula.clauses[i];
        /* find and skip any clause already has a satisfied literal */
        if(clause.assign_satisfacctions(literal_assignments)){
            continue;
        }

        for(int j = 0; j < clause.literals.size(); j++)
        {
            int literal = clause.literals[j];
            /* record the number of times each unassigned literal appears in an unsatisfied clause */
            if(literal_assignments.get_literal_assignment(literal) == UNASSIGNED)
            {
                if(literal > 0)
                {
                    dlis_counter.positive_count[literal]++;
                }
                else
                {
                    dlis_counter.negative_count[std::abs(literal)]++;
                }
            }
        }
    }

    /* find the new best literal to pick, complemented or not */
    for(int i = 1; i <= formula.num_vars; i++)
    {
        if(literal_assignments.tracking_assignments[i] == UNASSIGNED)
        {
            /*compare againts both counters*/
            if(dlis_counter.positive_count[i] > best_counter)
            {
                best_counter = dlis_counter.positive_count[i];
                new_literal = i;
            }
            
            if(dlis_counter.negative_count[i] > best_counter)
            {
                best_counter = dlis_counter.negative_count[i];
                new_literal = -i;
            }
            
        }
    }
    return new_literal;
}

/**
 * Backtraking function to remove multiple assigments, may be overkill if no additional hueristics are implemented
 * @param Literal_Assignments&: reference to the structure that holds all the literal assignments, which will be updated during backtracking
 * @param int: the size of the trail before the current branching, used to determine how
 * @return void
 */
void backtrack(Literal_Assignments& literal_assignments, int size_status) {
    for(int i = literal_assignments.get_trail_assignments_size(); i > size_status; i--){
        literal_assignments.unassign_last_literal();
    }
    return;
}


/**
 * Function that performs the DPLL algorithm with two watched literals and DLIS branching heuristic.
 * @param CNF_Formula&: reference to the CNF formula for which to perform DPLL
 * @param All_Watched_Literals&: reference to the structure that holds all the watched literals
 * @param Literal_Assignments&: reference to the structure that holds all the literal assignments
 * @return bool: returns if CNF formula is satisfiable or not
 */
bool DPLL(CNF_Formula& formula, All_Watched_Literals& awl, Literal_Assignments& literal_assignments){

    /* PRECONDITION: base case, look for for unit clauses */
    if(BCP(formula, awl, literal_assignments) == false){
        return false; 
    }

    /* Branching Setup: Pick literal to branch to */
    int new_branching_literal;
    if(DLSI_INHIBITED){
        new_branching_literal = basic_brancher(formula, literal_assignments);
    }
    else{
        new_branching_literal = DLIS(formula, literal_assignments);
    }

    int size_status = (int)literal_assignments.get_trail_assignments_size();

    /* base case for when no unassigned literals remain */
    if(new_branching_literal == 0){
        return true;
    }

    /* Recrusion 1, branch based on new assignment */
    awl.unit_propagation_queue.push_back(new_branching_literal);    /* add new chosen literal to queue and do BCP */
    if(DPLL(formula, awl, literal_assignments) == true){
        return true; /* SAT found!*/
    }
    /* backtrack */
    backtrack(literal_assignments, size_status);
    awl.clear_unit_propagation_queue();


    /*Recrusion 2, branch based on new assignment*/
    awl.unit_propagation_queue.push_back(-new_branching_literal);  /* add new chosen literal to queue and do BCP */
    if(DPLL(formula, awl, literal_assignments) == true){
        return true; /* SAT found!*/
    }
    /* backtrack */
    backtrack(literal_assignments, size_status);
    awl.clear_unit_propagation_queue();

    return false; /* no solution found in either branch, return false to backtrack further up the search tree */
}

/**
 * Funtion to print assingments using project output format
 * @param std::vector<Assignment>: where the final literal assignments are located
 * @return void
 */
void print_assigments(const std::vector<Assignment>& final_literal_assignments) {
    std::cout << "RESULT:SAT\n";
    std::string output = "ASSIGNMENT:";

    for(int i = 1; i < final_literal_assignments.size(); i++) // Start from 1 since index 0 is unused
    {

        if(final_literal_assignments[i] == ASSIGNED_TRUE) 
        {
            output += std::to_string(i) + "=1 "; // "variable=1 "
        }
        else if (final_literal_assignments[i] == ASSIGNED_FALSE) 
        {
            output += std::to_string(i) + "=0 "; // "variable=0 "
        }
    }
    output.pop_back(); /* Remove the trailing space */
    std::cout << output << "\n";
}

/**
 * main function
 * @param int: number of command line arguments
 * @param char**: array of command line arguments
 * @return int: executable return code
 */

int main(int argc, char* argv[]) {

    /*==============Verify Arguments and the Benchmark File================*/
    /* verify number of args given */
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

    /* parse benchmark file */
    Formula parsed_cnf_formula;
    if (!parse_dimacs(benchmark_file, parsed_cnf_formula)) 
    {
        /* failed to parse */
        std::cout << "RESULT:UNSAT\n";
        return 1;
    }

    /*==============Initiailize data structures for DPLL================*/

    /* create clauses, redundant but used to seperate AI generated code to my own driven development/design */
    CNF_Formula my_Formula= initialize_cnf_formula(parsed_cnf_formula);

    /* initialize watched literals data structure */
    All_Watched_Literals watched_literals;
    if(!initialize_watched_literals(my_Formula, watched_literals))
    {
        std::cout << "RESULT:UNSAT\n";
        return 1;
    }

    /*==============Run SAT Solver algorithm================*/
    /* run dpll*/
    Literal_Assignments literal_assignments(my_Formula.num_vars); /* call contructor for literal assignments */

    bool is_satisfiable = DPLL(my_Formula, watched_literals, literal_assignments); 



    /*==============Print Output================*/
    if (is_satisfiable) 
    {
        print_assigments(literal_assignments.get_final_literal_assignment()); //TODO update to align with final prinout formart
    }
    else
    {
        std::cout << "RESULT:UNSAT\n";
    }

    return 0;
}
