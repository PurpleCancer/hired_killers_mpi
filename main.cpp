#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mpi.h>
#include <ctime>
#include <vector>
#include <pthread.h>
#include "CompanyRequest.hpp"
#include "Company.hpp"
#include "Definitions.hpp"

using namespace std;

// data shared between threads

int numberOfCompanies;
vector<Company *> companies;
int setFlags;

int lamportClock;
vector<int> lamportVector;
bool lamportVectorChangeFlag;

pthread_mutex_t lamportClockMutex   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lamportVectorMutex  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t setFlagsMutex       = PTHREAD_MUTEX_INITIALIZER;
vector<pthread_mutex_t> companyMutex;

// function of the message receiving thread
void *receiverFunction(void * arg)
{
    message_data data;
    MPI_Status status;

    while(true)
    {
        MPI_Recv(&data, sizeof(message_data), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch (status.MPI_TAG)
        {
            case MSG_QUEUE:
                pthread_mutex_lock(&lamportClockMutex);
                pthread_mutex_lock(&lamportVectorMutex);
                pthread_mutex_lock(&companyMutex[data.companyId]);
                pthread_mutex_lock(&setFlagsMutex);

                if (data.lamportClock > lamportClock)
                    lamportClock = data.lamportClock;
                lamportClock++;

                lamportVector[status.MPI_SOURCE] = data.lamportClock;
                
                companies[data.companyId]->addToQueue(*(new CompanyRequest(data.lamportClock, status.MPI_SOURCE)));
                if (companies[data.companyId]->getFlag())
                    setFlags++;
                companies[data.companyId]->setFlag(true);

                lamportVectorChangeFlag = true;

                data.lamportClock = ++lamportClock;
                MPI_Send(&data, sizeof(message_data), MPI_BYTE, status.MPI_SOURCE, MSG_ACK, MPI_COMM_WORLD);

                pthread_mutex_unlock(&setFlagsMutex);
                pthread_mutex_unlock(&companyMutex[data.companyId]);
                pthread_mutex_unlock(&lamportVectorMutex);
                pthread_mutex_unlock(&lamportClockMutex);
                break;
            case MSG_DEQUEUE:
                pthread_mutex_lock(&lamportClockMutex);
                pthread_mutex_lock(&lamportVectorMutex);
                pthread_mutex_lock(&companyMutex[data.companyId]);
                pthread_mutex_lock(&setFlagsMutex);

                if (data.lamportClock > lamportClock)
                    lamportClock = data.lamportClock;
                lamportClock++;

                lamportVector[status.MPI_SOURCE] = data.lamportClock;

                companies[data.companyId]->removeFromQueue(status.MPI_SOURCE);
                if (companies[data.companyId]->getFlag())
                    setFlags++;
                companies[data.companyId]->setFlag(true);

                lamportVectorChangeFlag = true;

                pthread_mutex_unlock(&setFlagsMutex);
                pthread_mutex_unlock(&companyMutex[data.companyId]);
                pthread_mutex_unlock(&lamportVectorMutex);
                pthread_mutex_unlock(&lamportClockMutex);
                break;
            case MSG_TAKE:
                pthread_mutex_lock(&lamportClockMutex);
                pthread_mutex_lock(&lamportVectorMutex);
                pthread_mutex_lock(&companyMutex[data.companyId]);
                pthread_mutex_lock(&setFlagsMutex);

                if (data.lamportClock > lamportClock)
                    lamportClock = data.lamportClock;
                lamportClock++;

                lamportVector[status.MPI_SOURCE] = data.lamportClock;

                companies[data.companyId]->takeKiller(status.MPI_SOURCE);
                if (companies[data.companyId]->getFlag())
                    setFlags++;
                companies[data.companyId]->setFlag(true);

                lamportVectorChangeFlag = true;

                pthread_mutex_unlock(&setFlagsMutex);
                pthread_mutex_unlock(&companyMutex[data.companyId]);
                pthread_mutex_unlock(&lamportVectorMutex);
                pthread_mutex_unlock(&lamportClockMutex);
                break;
            case MSG_RETURN:
                pthread_mutex_lock(&lamportClockMutex);
                pthread_mutex_lock(&lamportVectorMutex);
                pthread_mutex_lock(&companyMutex[data.companyId]);
                pthread_mutex_lock(&setFlagsMutex);

                if (data.lamportClock > lamportClock)
                    lamportClock = data.lamportClock;
                lamportClock++;

                lamportVector[status.MPI_SOURCE] = data.lamportClock;

                companies[data.companyId]->returnKiller();
                companies[data.companyId]->rate(data.rating);
                if (companies[data.companyId]->getFlag())
                    setFlags++;
                companies[data.companyId]->setFlag(true);

                lamportVectorChangeFlag = true;

                pthread_mutex_unlock(&setFlagsMutex);
                pthread_mutex_unlock(&companyMutex[data.companyId]);
                pthread_mutex_unlock(&lamportVectorMutex);
                pthread_mutex_unlock(&lamportClockMutex);
                break;
            case MSG_ACK:
                pthread_mutex_lock(&lamportClockMutex);
                pthread_mutex_lock(&lamportVectorMutex);
                pthread_mutex_lock(&setFlagsMutex);

                if (data.lamportClock > lamportClock)
                    lamportClock = data.lamportClock;
                lamportClock++;

                lamportVector[status.MPI_SOURCE] = data.lamportClock;

                lamportVectorChangeFlag = true;

                pthread_mutex_unlock(&setFlagsMutex);
                pthread_mutex_unlock(&lamportVectorMutex);
                pthread_mutex_unlock(&lamportClockMutex);
                break;
            default:
                cout<<"Bad message tag."<<endl;
                break;
        }
    }

    return NULL;
}


int main(int argc, char ** argv)
{
    int size, rank;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    lamportClock = 0;
    for (int i = 0; i < size; ++i)
        lamportVector.push_back(0);

    lamportClockMutex   = PTHREAD_MUTEX_INITIALIZER;
    lamportVectorMutex  = PTHREAD_MUTEX_INITIALIZER;

    pthread_t receiver_thread;
    

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
            companyMutex.push_back(PTHREAD_MUTEX_INITIALIZER);
        }

        setFlags = numberOfCompanies;
        lamportVectorChangeFlag = true;

        pthread_create(&receiver_thread, NULL, &receiverFunction, NULL);

        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    //Algorithm


    MPI_Finalize();
    
    return 0;
}