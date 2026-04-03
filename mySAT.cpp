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
 * main function
 * @params argc: number of command line arguments
 * @params argv: array of command line arguments
 */

int main(int argc, char* argv[]) {

    /* verify number of args given*/
    if (argc != 2)
    {
        std::cout << "1RESULT:UNSAT\n";
        return 1;
    }

    std::string benchmark_file;
    benchmark_file = argv[1];

    /* check if a valid file was given*/
    if(benchmark_file.empty())
    {
        std::cout << "2RESULT:UNSAT\n";
        return 1;
    }

    /* parse benchmark file */
    Formula cnf_formula;
    if (!parse_dimacs(benchmark_file, cnf_formula)) 
    {
        /* failed to parse */
        std::cout << "3RESULT:UNSAT\n";
        return 1;
    }
    else
    {
        /* successfully parsed */
        std::cout << "did parsing\n";
    }




    return 1;
}
