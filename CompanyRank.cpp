#include "CompanyRank.hpp"

CompanyRank::CompanyRank(int id, double rating)
{
    this->companyId = id;
    this->rating = rating;
}

bool CompanyRank::operator < (const CompanyRank &rank)
{
    return (this->rating < rank.rating);
}

bool CompanyRank::operator > (const CompanyRank &rank)
{
    return (this->rating > rank.rating);
}

double CompanyRank::getRating()
{
    return this->rating;
}

int CompanyRank::getCompanyId()
{
    return this->companyId;
}