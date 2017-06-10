#include "CompanyRequest.hpp"
#include "Company.hpp"

Company::Company(int killers)
{
    this->numberOfKillers = killers;

    srand(time(0));
    this->numberOfRatings = 0;
    this->averageRating = 0;
    int targetRatings = rand()%(MAX_NO_OF_STARTING_RATINGS - 1) + 1;
    for (int i = 0; i < targetRatings; ++i)
    while (this->numberOfRatings < targetRatings)
    {
        int rating = rand()%5 + 1;
        this->averageRating = this->averageRating + \
            (((double)rating - this->averageRating)/((double)++this->numberOfRatings));
    }
}

Company::Company(int killers, int seed)
{
    this->numberOfKillers = killers;

    srand(seed);
    this->numberOfRatings = 0;
    this->averageRating = 0;
    int targetRatings = rand()%(MAX_NO_OF_STARTING_RATINGS - 1) + 1;
    for (int i = 0; i < targetRatings; ++i)
    while (this->numberOfRatings < targetRatings)
    {
        int rating = rand()%5 + 1;
        this->averageRating = this->averageRating + \
            (((double)rating - this->averageRating)/((double)++this->numberOfRatings));
    }
}

double Company::getRating()
{
    return this->averageRating;
}

void Company::rate(int rating)
{
    this->averageRating = this->averageRating + \
            (((double)rating - this->averageRating)/((double)++this->numberOfRatings));
}

int Company::getKillers()
{
    return this->numberOfKillers;
}

void Company::takeKiller(int processId)
{
    for (list<CompanyRequest>::iterator it = requestsList.begin(); it != requestsList.end(); ++it)
    {
        if ((*it).getProcessId() == processId)
        {
            requestsList.erase(it);
            break;
        }
    }

    this->numberOfKillers--;
}

void Company::addToQueue(CompanyRequest request)
{
    for (list<CompanyRequest>::iterator it = requestsList.begin(); it != requestsList.end(); ++it)
    {
        if (request < (*it))
        {
            requestsList.insert(it, request);
            return;
        }
    }

    requestsList.push_back(request);
}

int Company::getQueuePosition(int processId)
{
    int i = 1;
    for (list<CompanyRequest>::iterator it = requestsList.begin(); it != requestsList.end(); ++it, ++i)
    {
        if ((*it).getProcessId() == processId)
        {
            return i;
        }
    }
    return -1;
}

bool Company::getFlag()
{
    return this->flag;
}

void Company::setFlag(bool flag)
{
    this->flag = flag;
}