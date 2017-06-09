#ifndef COMPANYREQUEST_H
#define COMPANYREQUEST_H

class CompanyRequest 
{
    int ClockValue;
    int ProcessId;

public:
    CompanyRequest(int clock, int pid);
    bool operator < (const CompanyRequest & req);
    bool operator > (const CompanyRequest & req);
    int getProcessId();
};

#endif