#include <cstdio>
#include <iostream>
#include <mpi.h>
#include "CompanyRequest.hpp"
#include "Company.hpp"

using namespace std;

int main(int argc, char ** argv)
{
    int size, rank;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    cout<<"Hello world!"<<endl;

    MPI_Finalize();
    
    return 0;
}