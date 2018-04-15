#include<stdio.h>
#include"systemRunner.h"
#include"helper.h"

void initRunFrame(struct systemContext* context)
{
    struct Window* runwin = context->runWindow;
    int i,maxResourceVal=0;
    for(i=0;i<context->bankerData->availableResourcesCount;++i)
        maxResourceVal = (maxResourceVal>context->bankerData->maxResourcesArray[i])? context->bankerData->maxResourcesArray[i]:maxResourceVal;
    UIframe_init(runwin->frame,calcRunWindowWidth(context->bankerData->processCount,maxResourceVal,context->bankerData->availableResourcesCount),20,'|','|','-','-', 1, 1);
}

void start(struct systemContext* context)
{
    char **optionsArray = (char*[]){"Run.","View Thread Behaviour.","Exit."};
    struct BankerData* bd = context->bankerData;
    struct UITextField* tf = &context->mainWindow->textFieldsArray[(bd->availableResourcesCount*(bd->processCount+1)*2)];
    printf("\tLoading...\n");
    printf("\tPreparing Main Screen...\n");
    prepareMainScreen(context->mainWindow,context);
    printf("\tPreparing Thread Behaviour Data Screen...\n");
    prepareThreadDataScreen(context->threadDataWindow,context);
    UIframe_nextLine(context->mainWindow->frame);
    UIframe_addLine(context->mainWindow->frame,"Use the arrow keys to navigate, press enter to select.",0,0.1);
    appendOptionsMenuToWindow(context->mainWindow,optionsArray,3,0,tf);
    int loop=1;
    while(loop)
    {
        clrscr();
        printMainScreen(context,10,1);
        int ch=navigateOptionsMenu(tf,3);
        switch(ch)
        {
            case 0: 
            // Run the banker's algorithm simulation using the provided context.(Asynchronously)
                run(context,0);
            break;
            case 1:
                clrscr();
                printThreadDataScreen(context,5,1);
            break;
            case 2:
            loop = 0;
            break;
        }
    }
}

int main()
{
    struct systemContext context;
    struct BankerData bd;
    const int processCount = 4, resourceCount = 3;
    int initcode = Banker_init(&bd,resourceCount,processCount,
        //MaxResourceArray
        (int[]){4, 4, 50},
        //resourcesDemandMatrix
        (int*[]){
            (int[]){1,1,40},
            (int[]){3,3,40},
            (int[]){1,1,40},
            (int[]){3,3,40}
            },
        //resourcesAllocatedMatrix
        (int*[]){
            (int[]){0,0,0},
            (int[]){0,0,0},
            (int[]){0,0,0},
            (int[]){0,0,0}
            }
        );
    // Add the BankerData to the context.
    context.bankerData = &bd;

    // Create the main window and add it to the context.
    struct Window mainwin;
    // Create a frame for the main window.
    struct UIframe mainframe;
    UIframe_init(&mainframe,96,30,'|','|','-','-',1,1);
    mainwin.frame = &mainframe;
    mainwin.textFieldCount = (processCount+1)*resourceCount*2 + 5 + 4;
    struct UITextField maintfarray[(processCount+1)*resourceCount*2 + 5 + 4];
    mainwin.textFieldsArray = maintfarray;
    context.mainWindow = &mainwin;

    // Create the threadData window...
    struct Window tdwin;
    struct UIframe tdframe;
    UIframe_init(&tdframe,64,30,'|','|','-','-',1,1);
    tdwin.frame = &tdframe;
    struct UITextField tdfieldarray[3*1000];
    tdwin.textFieldsArray = tdfieldarray;
    tdwin.textFieldCount = 3000;
    context.threadDataWindow = &tdwin;

    // Create the run Window and add it to context.
    struct Window runwin;
    context.runWindow = &runwin;
    // Create a frame for the run window.
    struct UIframe runframe;
    context.runWindow->frame = &runframe;
    initRunFrame(&context);
    //Initialize the run window's text field array. The run window has 7 text fields for each process + 1 text field for current banker data.
    struct UITextField runtfarray[processCount*7+2];
    context.runWindow->textFieldsArray = runtfarray;
    context.runWindow->textFieldCount = processCount*7+2;

    // Create threadTickBehaviour data array.
    struct threadTickBehaviour tb[processCount];
    // Create semaphores required for thread synchronisation.
    sem_t twaiter[processCount],onTickSem;
    // Initialize global semaphore.
    sem_init(&onTickSem, 0, 0);

    // Initialize thread private semaphores and their threadTickBehaviour data strutures.
    int i;
    for(i=0;i<processCount;++i){
        sem_init(&twaiter[i], 0, 0);
        // Initialize same tick behaviour for all threads.
        initTickBehaviour(&(tb[i]), 1, 0, 5, (int*[]){(int[]){2,10},(int[]){0,1},(int[]){2,-10},(int[]){2,0},(int[]){2,10}}, &twaiter[i], &onTickSem, i, context.bankerData);
    }

    // Add tick data, thread count and global semaphore to the system context.    
    context.tickBehaviourArray = tb;
    context.threadCount = i;
    context.onTickSem = &onTickSem;

    start(&context);
}