#include <openmpi/mpi.h>
#include <cstdio>
#include <cstdlib>
#include "CriticalSectionRequest.h"
#include "symbols.h"
#include <vector>
#include <unistd.h>
#include <list>

int *requestResponses;
int size;
int logicalClock;
int targetedCriticalSection;
int *tempTable;
int *incomingMessage;
MPI_Status status;
MPI_Request mpiRequest;

int criticalSectionsQuantity;
int secondsInCriticalSection;
int rank;
std::list<CriticalSectionRequest *> requestQueue;

CriticalSectionRequest *incomingRequest;


int totalRuns = 1;


CriticalSectionRequest *criticalSectionRequest;


bool wayToGo() {
    for (int i = 0; i < size; i++) {
        if (requestResponses[i] != OK)
            return false;
    }

    return true;
}

void clearResponses() {
    for (int i = 0; i < size; i++) {
        requestResponses[i] = NO_VALUE;
    }
}


void chooseCriticalSection() {
    //random critical section
    targetedCriticalSection = rand() % criticalSectionsQuantity;

    printf("%d: targeting section no %d\n", rank, targetedCriticalSection);
}

void broadcastRequest() {
    criticalSectionRequest = new CriticalSectionRequest(rank, logicalClock,
                                                        targetedCriticalSection);
    tempTable = criticalSectionRequest->pack();

    // add own request to queue, until getting all acks
    requestQueue.push_back(criticalSectionRequest);

    //approve own request
    requestResponses[rank] = OK;

    //broadcast request
    for (int i = 0; i < size; i++) {
        if (i != rank) {
            MPI_Send(tempTable, 3, MPI_INT, i, REQUEST, MPI_COMM_WORLD);
        }
    }

    //broadcast is considered as one operation
    logicalClock++;
}


void updateLogicalClock() {
    //update Lamport logical clock
    if (incomingRequest->getRequesterLogicalClock() > logicalClock) {
        logicalClock = incomingRequest->getRequesterLogicalClock() + 1;
    } else {
        logicalClock++;
    }
}

void sendConfirmation(int receiver) {
    int *confirmation = new int[3];
    CriticalSectionRequest *confObject = new CriticalSectionRequest();
    confirmation = confObject->pack();

    MPI_Send(confirmation, 3, MPI_INT, receiver, OK, MPI_COMM_WORLD);
    logicalClock++;
    printf("%d: sent ACK to %d\n", rank, receiver);


    //delete [] confirmation;
    delete confObject;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: \nmpirun -np <#processes quantity> <critical sections "
                       "quantity> <seconds to spend in criticalsection>\n"
                       "[<critical section request quantity>] - default is 1");
        exit(-1);
    }

    if (argc >= 4) {
        totalRuns = atoi(argv[3]);
    }

    int len;
    char processor[100];
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(processor, &len);

    secondsInCriticalSection = atoi(argv[2]);
    criticalSectionsQuantity = atoi(argv[1]);

    if (criticalSectionsQuantity >= size) {
        printf("There's more critical sections than processes, so this program makes no sense. Good bye!\n");
        exit(-1);
    }

    // what is the best seed?
    srand(size * rank * getpid());


    requestResponses = new int[size];

    // left initialization of clock with 0 value
    logicalClock = 0;

    tempTable = new int[3];
    incomingMessage = new int[3];


    //repeating request to critical section given times
    for (int i = 0; i < totalRuns; i++) {
        chooseCriticalSection();
        broadcastRequest();


        //receiving confirmations and other requests
        while (!wayToGo()) {

            MPI_Recv(incomingMessage, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            //rejecting messages from myself
            if (status.MPI_SOURCE != rank) {
                if (status.MPI_TAG == OK) {
                    //confirmation
                    requestResponses[status.MPI_SOURCE] = status.MPI_TAG;
                    printf("%d: OK from %d\n", rank, status.MPI_SOURCE);
                } else {
                    printf("%d: request from %d\n", rank, status.MPI_SOURCE);
                    //request
                    incomingRequest = new CriticalSectionRequest(incomingMessage[0],
                                                                 incomingMessage[1],
                                                                 incomingMessage[2]);

                    updateLogicalClock();


                    //sending confirmation if:
                    //incoming request is about other critical section
                    //or if critical sections are the same, but my clock is lower, so I have precedence
                    //or if critical sections are the same and clock are the same, wins older process (with lower rank)

                    if (incomingRequest->getCriticalSectionNumber() != targetedCriticalSection ||
                        criticalSectionRequest->getRequesterLogicalClock() >
                        incomingRequest->getRequesterLogicalClock() ||
                        incomingRequest->getRequesterRank() > rank) {

                        sendConfirmation(status.MPI_SOURCE);
                    } else {
                        requestQueue.push_back(incomingRequest);
                    }
                }
            }
        }



        //go into critical section
        printf("%d: goes to critical section no. %d\n", rank, targetedCriticalSection);
        fflush(stdin);
        sleep(secondsInCriticalSection);

        requestQueue.remove(criticalSectionRequest);
        clearResponses();

        printf("%d: out of critical section no. %d\n", rank, targetedCriticalSection);
        fflush(stdin);


        for (CriticalSectionRequest *request : requestQueue) {
            sendConfirmation(request->getRequesterRank());
        }

        requestQueue.clear();
    }

    delete[] requestResponses;
    delete[] tempTable;
    delete[] incomingMessage;
    MPI_Finalize();
}