#ifndef __SYSTEMRUNNER_C__
#define __SYSTEMRUNNER_C__

#include<semaphore.h>
#include"systemContextHelper.h"

struct threadTickBehaviour
{
    // No. of ticks the process lives for and the current tick the process is at.
    int lifeTicks, currentTick;

    // If this is 0, thread does not wait or post on semaphores.
    int isThreadSynchronised;

    // The thread sleeps for this time before requesting the resource again.
    int requestWaitInterval;

    // The thread waits on this semaphore before requesting resources again.
    sem_t* tickWaiter;
    // This semaphore is posted on when the thread finishes a tick.
    sem_t* onTickSem;

    // The process index for this thread to be used by the banker.
    int processIndex;

    // The banker to release or demand resources from.
    struct BankerData* banker;

    // The response code of the last request to the banker
    int lastReqResponse;

    // Pointer to array of two integers.
    // its size should be [lifeTicks][2]
    // Stores what resource and its count to be demanded or released per tick.
    // -ve values of count mean release resource, +ve count means demand resource.
    int **perTickResourceDemandMatrix;
};

void initTickBehaviour(struct threadTickBehaviour *tb, int requestWaitInterval, int isThreadSynchronised, int lifeTicks, int **perTickResourceDemandMatrix, sem_t *tickWaiter, sem_t *onTickSem, int processIndex, struct BankerData *banker);
void run(struct systemContext* context, int synchronised);
void *threadFunction(void *tickBehaviour);

#endif // __SYSTEMRUNNER_C__