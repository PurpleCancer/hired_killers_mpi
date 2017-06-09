#ifndef COMPANYREQUEST_H
#define COMPANYREQUEST_H

class CompanyRequest 
{
    int ClockValue;
    int ProccessId;

public:
    CompanyRequest(int clock, int pid);
    bool operator < (const CompanyRequest & req);
    bool operator > (const CompanyRequest & req);
};

#endif