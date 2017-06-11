#ifndef COMPANYRANK_H
#define COMPANYRANK_H

class CompanyRank
{
    int companyId;
    double rating;

public:
    CompanyRank(int id, double rating);
    bool operator < (const CompanyRank &rank);
    bool operator > (const CompanyRank &rank);
    double getRating();
    int getCompanyId();
};

#endif