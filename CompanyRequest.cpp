#include "CompanyRequest.hpp"

CompanyRequest::CompanyRequest(int clock, int pid)
{
    this->ClockValue = clock;
    this->ProccessId = pid;
}

bool CompanyRequest::operator < (const CompanyRequest & req)
{
    if (req.ClockValue == this->ClockValue)
        return (this->ProccessId < req.ProccessId);
    else
        return (this->ClockValue < req.ClockValue);
}

bool CompanyRequest::operator > (const CompanyRequest & req)
{
    if (req.ClockValue == this->ClockValue)
        return (this->ProccessId > req.ProccessId);
    else
        return (this->ClockValue > req.ClockValue);
}