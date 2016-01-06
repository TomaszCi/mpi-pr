//
// Created by tomasz on 12/13/15.
//

#include "CriticalSectionRequest.h"

int* CriticalSectionRequest::pack() {
    tab[0] = requesterRank;
    tab[1] = requesterLogicalClock;
    tab[2] = criticalSectionNumber;
    return tab;
}

CriticalSectionRequest::CriticalSectionRequest() {
    requesterRank = NO_VALUE;
    requesterLogicalClock = NO_VALUE;
    criticalSectionNumber = NO_VALUE;
    tab = new int[3];
}

CriticalSectionRequest::~CriticalSectionRequest() {
    delete [] tab;
}
