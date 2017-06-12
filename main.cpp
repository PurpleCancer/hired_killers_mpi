#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mpi.h>
#include <ctime>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <algorithm>
#include "CompanyRequest.hpp"
#include "Company.hpp"
#include "CompanyRank.hpp"
#include "Definitions.hpp"

using namespace std;

// data shared between threads
int size, mpi_rank;

int numberOfCompanies;
vector<Company *> companies;
int setFlags;

int lamportClock;
vector<int> lamportVector;
bool lamportVectorChangeFlag;

pthread_mutex_t lamportClockMutex;
pthread_mutex_t lamportVectorMutex;
pthread_mutex_t setFlagsMutex;
pthread_mutex_t companyMutex;
// vector<pthread_mutex_t> companyMutex;

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
                pthread_mutex_lock(&companyMutex);
                pthread_mutex_lock(&setFlagsMutex);
                pthread_mutex_lock(&lamportClockMutex);
                pthread_mutex_lock(&lamportVectorMutex);

                if (data.lamportClock > lamportClock)
                    lamportClock = data.lamportClock;
                lamportClock++;

                lamportVector[status.MPI_SOURCE] = data.lamportClock;
                
                companies[data.companyId]->addToQueue(*(new CompanyRequest(data.lamportClock, status.MPI_SOURCE)));
                if (!companies[data.companyId]->getFlag())
                    setFlags++;
                companies[data.companyId]->setFlag(true);

                lamportVectorChangeFlag = true;

                data.lamportClock = ++lamportClock;
                MPI_Send(&data, sizeof(message_data), MPI_BYTE, status.MPI_SOURCE, MSG_ACK, MPI_COMM_WORLD);

                pthread_mutex_unlock(&lamportVectorMutex);
                pthread_mutex_unlock(&lamportClockMutex);
                pthread_mutex_unlock(&setFlagsMutex);
                pthread_mutex_unlock(&companyMutex);
                break;
            case MSG_DEQUEUE:
                pthread_mutex_lock(&companyMutex);
                pthread_mutex_lock(&setFlagsMutex);
                pthread_mutex_lock(&lamportClockMutex);
                pthread_mutex_lock(&lamportVectorMutex);

                if (data.lamportClock > lamportClock)
                    lamportClock = data.lamportClock;
                lamportClock++;

                lamportVector[status.MPI_SOURCE] = data.lamportClock;

                companies[data.companyId]->removeFromQueue(status.MPI_SOURCE);
                if (!companies[data.companyId]->getFlag())
                    setFlags++;
                companies[data.companyId]->setFlag(true);

                lamportVectorChangeFlag = true;

                pthread_mutex_unlock(&lamportVectorMutex);
                pthread_mutex_unlock(&lamportClockMutex);
                pthread_mutex_unlock(&setFlagsMutex);
                pthread_mutex_unlock(&companyMutex);
                break;
            case MSG_TAKE:
                pthread_mutex_lock(&companyMutex);
                pthread_mutex_lock(&setFlagsMutex);
                pthread_mutex_lock(&lamportClockMutex);
                pthread_mutex_lock(&lamportVectorMutex);

                if (data.lamportClock > lamportClock)
                    lamportClock = data.lamportClock;
                lamportClock++;

                lamportVector[status.MPI_SOURCE] = data.lamportClock;

                companies[data.companyId]->takeKiller(status.MPI_SOURCE);
                if (!companies[data.companyId]->getFlag())
                    setFlags++;
                companies[data.companyId]->setFlag(true);

                lamportVectorChangeFlag = true;

                pthread_mutex_unlock(&lamportVectorMutex);
                pthread_mutex_unlock(&lamportClockMutex);
                pthread_mutex_unlock(&setFlagsMutex);
                pthread_mutex_unlock(&companyMutex);
                break;
            case MSG_RETURN:
                pthread_mutex_lock(&companyMutex);
                pthread_mutex_lock(&setFlagsMutex);
                pthread_mutex_lock(&lamportClockMutex);
                pthread_mutex_lock(&lamportVectorMutex);

                if (data.lamportClock > lamportClock)
                    lamportClock = data.lamportClock;
                lamportClock++;

                lamportVector[status.MPI_SOURCE] = data.lamportClock;

                companies[data.companyId]->returnKiller();
                companies[data.companyId]->rate(data.rating);
                if (!companies[data.companyId]->getFlag())
                    setFlags++;
                companies[data.companyId]->setFlag(true);

                lamportVectorChangeFlag = true;

                pthread_mutex_unlock(&lamportVectorMutex);
                pthread_mutex_unlock(&lamportClockMutex);
                pthread_mutex_unlock(&setFlagsMutex);
                pthread_mutex_unlock(&companyMutex);
                break;
            case MSG_ACK:
                pthread_mutex_lock(&companyMutex);
                pthread_mutex_lock(&setFlagsMutex);
                pthread_mutex_lock(&lamportClockMutex);
                pthread_mutex_lock(&lamportVectorMutex);

                if (data.lamportClock > lamportClock)
                    lamportClock = data.lamportClock;
                lamportClock++;

                lamportVector[status.MPI_SOURCE] = data.lamportClock;

                if (!companies[data.companyId]->getFlag())
                    setFlags++;
                companies[data.companyId]->setFlag(true);

                lamportVectorChangeFlag = true;

                pthread_mutex_unlock(&lamportVectorMutex);
                pthread_mutex_unlock(&lamportClockMutex);
                pthread_mutex_unlock(&setFlagsMutex);
                pthread_mutex_unlock(&companyMutex);
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
    // MPI_Init(&argc, &argv);
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided != MPI_THREAD_MULTIPLE)
    {
        cout<<"No multithreading MPI support."<<endl;
        return -1;
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    lamportClock = 0;
    for (int i = 0; i < size; ++i)
        lamportVector.push_back(0);

    lamportClockMutex   = PTHREAD_MUTEX_INITIALIZER;
    lamportVectorMutex  = PTHREAD_MUTEX_INITIALIZER;
    setFlagsMutex       = PTHREAD_MUTEX_INITIALIZER;
    companyMutex        = PTHREAD_MUTEX_INITIALIZER;

    pthread_t receiver_thread;
    

    // Initialization
    {    
        int randomSeed;
        // Randomize initial values
        if (mpi_rank == ROOT)
        {
            srand(time(0));
            //srand(12345);
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
            if (mpi_rank == 0)
                printf("Firma %i, %i zabojcow\n", i, companies[i]->getKillers());
        }

        setFlags = numberOfCompanies;
        lamportVectorChangeFlag = true;

        pthread_create(&receiver_thread, NULL, &receiverFunction, NULL);

        MPI_Barrier(MPI_COMM_WORLD);
    }

    srand(time(0) + time(0));
    
    // Algorithm
    while (true)
    {
        vector<CompanyRank> rankedCompanies;

        {
            // sleep until a killer is needed
            unsigned int timeToSleep = rand()%(MAX_SLEEP_MSEC - MIN_SLEEP_MSEC + 1) + MIN_SLEEP_MSEC;
            usleep(timeToSleep);
            printf("Proces %i, zegar %i: potrzebuje zabojcy\n", mpi_rank, lamportClock);
        }
        {
            // create a sorted vector
            pthread_mutex_lock(&companyMutex);
            pthread_mutex_lock(&setFlagsMutex);

            for (int i = 0; i < numberOfCompanies; ++i)
            {
                rankedCompanies.push_back(*(new CompanyRank(i, companies[i]->getRating())));
                companies[i]->setFlag(true);
            }

            sort (rankedCompanies.begin(), rankedCompanies.end());
            reverse (rankedCompanies.begin(), rankedCompanies.end());

            setFlags = numberOfCompanies;

            pthread_mutex_unlock(&setFlagsMutex);
            pthread_mutex_unlock(&companyMutex);
        }

        // look for a company
        bool notFound = true;
        int killersCompany;
        while (notFound)
        {
            // go through the companies
            while (setFlags == 0)
                usleep(100000);

                // pthread_mutex_lock(&companyMutex[data.companyId]);
                // pthread_mutex_lock(&setFlagsMutex);
                // pthread_mutex_lock(&lamportClockMutex);
                // pthread_mutex_lock(&lamportVectorMutex);

            for (vector<CompanyRank>::iterator it = rankedCompanies.begin(); it != rankedCompanies.end(); ++it)
            {
                int companyId = it->getCompanyId();

                pthread_mutex_lock(&companyMutex);
                pthread_mutex_lock(&setFlagsMutex);
                
                // check the next company
                if (!companies[companyId]->getFlag())
                {
                    pthread_mutex_unlock(&setFlagsMutex);
                    pthread_mutex_unlock(&companyMutex);

                    continue;
                }
                
                if (companies[companyId]->getQueuePosition(mpi_rank) == -1)
                {
                    // queue up in this company
                    pthread_mutex_lock(&lamportClockMutex);
                    lamportClock++;

                    message_data data;
                    data.companyId = companyId;
                    data.lamportClock = lamportClock;

                    for (int i = 0; i < size; ++i)
                    {
                        if (i != mpi_rank)
                        {
                            MPI_Send(&data, sizeof(message_data), MPI_BYTE, i, MSG_QUEUE, MPI_COMM_WORLD);
                        }
                    }

                    companies[companyId]->addToQueue(*(new CompanyRequest(lamportClock, mpi_rank)));
                    // if (companies[companyId]->getFlag())
                    //     setFlags--;
                    // companies[companyId]->setFlag(false);


                    printf("Proces %i, zegar %i: kolejkuje sie u firmy %i\n", mpi_rank, lamportClock, companyId);

                    pthread_mutex_unlock(&lamportClockMutex);
                    pthread_mutex_unlock(&setFlagsMutex);        
                    pthread_mutex_unlock(&companyMutex);

                    break;
                }
                else
                {
                    pthread_mutex_lock(&lamportClockMutex);
                    pthread_mutex_lock(&lamportVectorMutex);

                    int companiesWithoutReply = 0;
                    for (int i = 0; i < size; ++i)
                    {
                        if (i != mpi_rank)
                        {
                            if (lamportVector[i] <= companies[companyId]->getQueueClock(mpi_rank))
                                companiesWithoutReply++;
                        }
                    }

                    if (companiesWithoutReply >= companies[companyId]->getKillers())
                    {
                        if (companies[companyId]->getFlag())
                            setFlags--;
                        companies[companyId]->setFlag(false);


                        pthread_mutex_unlock(&lamportVectorMutex);
                        pthread_mutex_unlock(&lamportClockMutex);
                        pthread_mutex_unlock(&setFlagsMutex);
                        pthread_mutex_unlock(&companyMutex);
                        break;
                    }
                    
                    // take killer
                    if (companies[companyId]->getQueuePosition(mpi_rank) < companies[companyId]->getKillers() - companiesWithoutReply + 1)
                    {
                        lamportClock++;
                        
                        message_data data;
                        data.companyId = companyId;
                        data.lamportClock = lamportClock;


                        for (int i = 0; i < size; ++i)
                        {
                            if (i != mpi_rank)
                            {
                                MPI_Send(&data, sizeof(message_data), MPI_BYTE, i, MSG_TAKE, MPI_COMM_WORLD);
                            }
                        }

                        killersCompany = companyId;

                        printf("Proces %i, zegar %i: wynajmuje zabojce od firmy %i\n", mpi_rank, lamportClock, killersCompany);

                        companies[companyId]->takeKiller(mpi_rank);

                        
                        // dequeue from other queues
                        for (int i = 0; i < numberOfCompanies; ++i)
                        {
                            if (companies[i]->getQueuePosition(mpi_rank) != -1)
                            {
                                lamportClock++;

                                data.companyId = i;
                                data.lamportClock = lamportClock;

                                for (int j = 0; j < size; ++j)
                                {
                                    if (j != mpi_rank)
                                    {
                                        MPI_Send(&data, sizeof(message_data), MPI_BYTE, j, MSG_DEQUEUE, MPI_COMM_WORLD);
                                    }
                                }
                                companies[i]->removeFromQueue(mpi_rank);
                            }
                        }

                        pthread_mutex_unlock(&lamportVectorMutex);
                        pthread_mutex_unlock(&lamportClockMutex);
                        pthread_mutex_unlock(&setFlagsMutex);
                        pthread_mutex_unlock(&companyMutex);

                        notFound = false;

                        break;
                    }

                    if (companies[companyId]->getQueuePosition(mpi_rank) <= 2)
                    {
                        message_data data;

                        for (vector<CompanyRank>::iterator it2 = it + 1; it2 != rankedCompanies.end(); ++it2)
                        {
                            if (companies[it2->getCompanyId()]->getQueuePosition(mpi_rank) != -1)
                            {
                                lamportClock++;

                                data.companyId = it2->getCompanyId();
                                data.lamportClock = lamportClock;

                                for (int j = 0; j < size; ++j)
                                {
                                    if (j != mpi_rank)
                                    {
                                        MPI_Send(&data, sizeof(message_data), MPI_BYTE, j, MSG_DEQUEUE, MPI_COMM_WORLD);
                                    }
                                }
                                companies[it2->getCompanyId()]->removeFromQueue(mpi_rank);


                                if (companies[it2->getCompanyId()]->getFlag())
                                    setFlags--;
                                companies[it2->getCompanyId()]->setFlag(false);


                                printf("Proces %i, zegar %i: opuszczam kolejke firmy %i\n", mpi_rank, lamportClock, it2->getCompanyId());
                            }
                        }


                        if (companies[companyId]->getFlag())
                            setFlags--;
                        companies[companyId]->setFlag(false);
                        
                        pthread_mutex_unlock(&lamportVectorMutex);
                        pthread_mutex_unlock(&lamportClockMutex);
                        pthread_mutex_unlock(&setFlagsMutex);
                        pthread_mutex_unlock(&companyMutex);

                        break;
                    }

                    pthread_mutex_unlock(&lamportVectorMutex);
                    pthread_mutex_unlock(&lamportClockMutex);
                    pthread_mutex_unlock(&setFlagsMutex);
                    pthread_mutex_unlock(&companyMutex);
                }

                
                if (companies[companyId]->getFlag())
                    setFlags--;
                companies[companyId]->setFlag(false);

                pthread_mutex_unlock(&setFlagsMutex);
                pthread_mutex_unlock(&companyMutex);
            }
        }

        // use a killer and return it
        {
            unsigned int timeToSleep = rand()%(MAX_SLEEP_MSEC - MIN_SLEEP_MSEC + 1) + MIN_SLEEP_MSEC;
            usleep(timeToSleep);


            pthread_mutex_lock(&companyMutex);
            pthread_mutex_lock(&setFlagsMutex);
            pthread_mutex_lock(&lamportClockMutex);
            pthread_mutex_lock(&lamportVectorMutex);

            lamportClock++;

            message_data data;
            int rating = rand()%5 + 1;

            data.companyId = killersCompany;
            data.lamportClock = lamportClock;
            data.rating = rating;

            for (int i = 0; i < size; ++i)
            {
                if (i != mpi_rank)
                {
                    MPI_Send(&data, sizeof(message_data), MPI_BYTE, i, MSG_RETURN, MPI_COMM_WORLD);
                }
            }
            companies[killersCompany]->returnKiller();
            companies[killersCompany]->rate(rating);

            pthread_mutex_unlock(&lamportVectorMutex);
            pthread_mutex_unlock(&lamportClockMutex);
            pthread_mutex_unlock(&setFlagsMutex);
            pthread_mutex_unlock(&companyMutex);


            printf("Proces %i, zegar %i: zwracam zabojce firmie %i\n", mpi_rank, lamportClock, killersCompany);
        }
    }



    MPI_Finalize();
    
    return 0;
}