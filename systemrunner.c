#include<stdio.h>
#include<stdlib.h>
#include<semaphore.h>
#include<string.h>
#include<pthread.h>
#include"banker.h"
#include"helper.h"
#include"UI.h"
#include"systemContextHelper.h"
#include"systemRunner.h"

void *threadFunction(void *tickBehaviour)
{
    struct threadTickBehaviour* behaviour = (struct threadTickBehaviour*)tickBehaviour;
    int resourceCount,currentTick=0;
    behaviour->currentTick=0;

    while(currentTick<behaviour->lifeTicks)
    {
        // If enabled, Wait on the semaphore for the next tick...
        if(behaviour->isThreadSynchronised)sem_wait(behaviour->tickWaiter);

        // Sleep for wait interval.
        if(behaviour->requestWaitInterval)sleep(behaviour->requestWaitInterval);

        resourceCount = behaviour->perTickResourceDemandMatrix[currentTick][1];
        // Allocate/deallocate resources.
        if(resourceCount>0)behaviour->lastReqResponse = Banker_requestResource(behaviour->banker,
                                                behaviour->processIndex,
                                                behaviour->perTickResourceDemandMatrix[currentTick][0],
                                                resourceCount);
        else behaviour->lastReqResponse = Banker_freeResource(behaviour->banker,
                                    behaviour->processIndex,
                                    behaviour->perTickResourceDemandMatrix[currentTick][0], -resourceCount);
        
        // Check if tick was successfull.
        if(behaviour->lastReqResponse == 1)
        {
            // Tick successfull, resource allocation/release done.
            behaviour->currentTick += 1;
            ++currentTick;
        }
        // Tick done, If enabled, post on semaphore to signal tick done
        if(behaviour->isThreadSynchronised)sem_post(behaviour->onTickSem);
    }
    // Release all resources.
    Banker_freeAllResources(behaviour->banker,behaviour->processIndex);
}
void initTickBehaviour(struct threadTickBehaviour *tb, int requestWaitInterval, int isThreadSynchronised, int lifeTicks, int **perTickResourceDemandMatrix, sem_t *tickWaiter, sem_t *onTickSem, int processIndex, struct BankerData *banker)
{
    tb->lifeTicks=lifeTicks;
    tb->banker = banker;
    tb->processIndex = processIndex;
    tb->tickWaiter = tickWaiter;
    tb->perTickResourceDemandMatrix = perTickResourceDemandMatrix;
    tb->onTickSem =onTickSem;
    tb->isThreadSynchronised = isThreadSynchronised;
    tb->requestWaitInterval = requestWaitInterval;
}
void run(struct systemContext* context, int synchronised)
{
    int ticksDone,newCount,runningThreadCount,noUpdateIteration;
    // Run the threads...
    // Create array of threads to run.
    pthread_t *userThreads = (pthread_t*)malloc(context->threadCount * sizeof(pthread_t));
    // Run the threads and pass them their tick behaviour structures.
    for(runningThreadCount=0;runningThreadCount<context->threadCount;++runningThreadCount)
        pthread_create(userThreads+runningThreadCount, NULL, threadFunction, context->tickBehaviourArray+runningThreadCount);

    // Prepare the current UIframe by laying out the TextFields.
    prepareRunWindow(context->runWindow,context->threadCount);
    // Clear the screen.
    clrscr();
    // Print the initial system state.
    printRunWindow(context,10,1);
    //UItextField_update(&context->runWindow->textFieldsArray[1],"Running Simulation...");
    if(synchronised)getch();

    // Loop while threads have work to do...
    while(runningThreadCount>0)
    {
        // If synchronisation enabled, do it.
        if(synchronised){
            // Resume all threads by allowing them to execute next tick.
            for(ticksDone=0;ticksDone<context->threadCount;++ticksDone)sem_post(context->tickBehaviourArray[ticksDone].tickWaiter);
            // Reset ticksDone
            ticksDone = 0;

            // Wait for the threads to finish their tick
            while(ticksDone<runningThreadCount)
            {
                // Wait for onTickSem to be posted by a thread...
                sem_wait(context->onTickSem);
                ++ticksDone;
            }
        }

        // Update threadCount regarding finished threads.
        runningThreadCount = 0;
        for(ticksDone=0;ticksDone<context->threadCount;++ticksDone)
            // Increment for unfinished threads.
            if(context->tickBehaviourArray[ticksDone].currentTick!=context->tickBehaviourArray[ticksDone].lifeTicks)
                ++runningThreadCount;

        // Print the current system state.
        printRunWindow(context,10,1);
        if(synchronised)getch();

        if(synchronised){
            // Check for deadlock, i.e no threads have valid lastReqResponse.
            // Reuse newCount and ticksDone variables...
            newCount = 1;
            for(ticksDone=0;ticksDone<context->threadCount;++ticksDone)
            {
                // If any running thread ticked without errors,i.e has status code 1, there is no deadlock...
                if(context->tickBehaviourArray[ticksDone].currentTick!= context->tickBehaviourArray[ticksDone].lifeTicks && (context->tickBehaviourArray[ticksDone].lastReqResponse == 1||context->tickBehaviourArray[ticksDone].lastReqResponse == -5))
                {
                    newCount = 0;
                    break;
                }
            }
            // Print errors on deadlock and break
            if(newCount)
            {
                ++noUpdateIteration;
                if(noUpdateIteration>1){
                    printf("\nOh no...! No thread proceded for two ticks, deadlock...!\n");
                    break;
                }
            }
            else noUpdateIteration = 0;
        }
    }
    // Free the thread memory.
    free(userThreads);
    UItextField_update(&context->runWindow->textFieldsArray[1],"Simulation ended, press any key to return.");
    getch();
}