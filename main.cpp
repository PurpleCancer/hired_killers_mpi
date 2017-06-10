#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mpi.h>
#include <ctime>
#include <vector>
#include "CompanyRequest.hpp"
#include "Company.hpp"
#include "Definitions.hpp"

using namespace std;

int main(int argc, char ** argv)
{
    int size, rank;

    int numberOfCompanies;
    vector<Company *> companies;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Initialization
    {    
        int randomSeed;
        // Randomize initial values
        if (rank == ROOT)
        {
            srand(time(0));
            if (NUMBER_OF_COMPANIES == 0)
                numberOfCompanies = rand()%(MAX_NUMBER_OF_COMPANIES - 1) + 1;
            else
                numberOfCompanies = NUMBER_OF_COMPANIES;

            randomSeed = rand();
        }

        MPI_Bcast(&numberOfCompanies, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
        MPI_Bcast(&randomSeed, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

        srand(randomSeed);

        for (int i = 0; i < numberOfCompanies; ++i)
        {
            int killers = rand()%(MAX_NUMBER_OF_KILLERS - MIN_NUMBER_OF_KILLERS + 1) + MIN_NUMBER_OF_KILLERS;
            int seed = rand();
            companies.push_back(new Company(killers, seed));
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    //Algorithm
    

    MPI_Finalize();
    
    return 0;
}