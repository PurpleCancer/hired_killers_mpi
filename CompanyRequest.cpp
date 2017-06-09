#include "CompanyRequest.hpp"

CompanyRequest::CompanyRequest(int clock, int pid)
{
    this->ClockValue = clock;
    this->ProcessId = pid;
}

bool CompanyRequest::operator < (const CompanyRequest & req)
{
    if (req.ClockValue == this->ClockValue)
        return (this->ProcessId < req.ProcessId);
    else
        return (this->ClockValue < req.ClockValue);
}

bool CompanyRequest::operator > (const CompanyRequest & req)
{
    if (req.ClockValue == this->ClockValue)
        return (this->ProcessId > req.ProcessId);
    else
        return (this->ClockValue > req.ClockValue);
}

int CompanyRequest::getProcessId()
{
    return this->ProcessId;
}