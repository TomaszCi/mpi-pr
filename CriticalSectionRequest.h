//
// Created by tomasz on 12/13/15.
//

#ifndef MPI_CRITICALSECTIONREQUEST_H
#define MPI_CRITICALSECTIONREQUEST_H

#include "symbols.h"

class CriticalSectionRequest {


public:

    int* pack();

    CriticalSectionRequest();

    CriticalSectionRequest(int requesterRank, int requesterLogicalClock, int criticalSectionNumber) : requesterRank(
            requesterRank), requesterLogicalClock(requesterLogicalClock), criticalSectionNumber(
            criticalSectionNumber) {
        tab = new int[3];
    }

    virtual ~CriticalSectionRequest();


    int getRequesterRank() const {
        return requesterRank;
    }

    int getRequesterLogicalClock() const {
        return requesterLogicalClock;
    }

    int getCriticalSectionNumber() const {
        return criticalSectionNumber;
    }

private:
    int requesterRank;
    int requesterLogicalClock;
    int criticalSectionNumber;
    int *tab;
};


#endif //MPI_CRITICALSECTIONREQUEST_H
