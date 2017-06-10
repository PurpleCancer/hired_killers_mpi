#ifndef COMPANY_H
#define COMPANY_H

#include <cstdlib>
#include <ctime>
#include <list>
#include "CompanyRequest.hpp"

#define MAX_NO_OF_STARTING_RATINGS 15

using namespace std;

class Company
{
    double averageRating;
    int numberOfRatings;
    int numberOfKillers;
    list<CompanyRequest> requestsList;
    bool flag;

public:
    Company(int killers);
    Company(int killers, int seed);
    double getRating();
    void rate(int rating);
    int getKillers();
    void takeKiller(int proccessId);
    void returnKiller();
    void addToQueue(CompanyRequest request);
    void removeFromQueue(int processId);
    int getQueuePosition(int processId);
    bool getFlag();
    void setFlag(bool flag);
};

#endif