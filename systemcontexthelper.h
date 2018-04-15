#ifndef __SYSTEMCONTEXTHELPER_C__
#define __SYSTEMCONTEXTHELPER_C__

#include<semaphore.h>
#include"UI.h"
#include"banker.h"
struct systemContext;
struct Window
{
    struct UIframe* frame;
    int textFieldCount;
    struct UITextField* textFieldsArray;
};
#include"systemRunner.h"
struct systemContext
{
    struct threadTickBehaviour *tickBehaviourArray;
    struct BankerData* bankerData;
    int threadCount;
    // This semaphore is posted on by each thread when the it finishes a tick.
    sem_t* onTickSem;

    struct Window* runWindow;
    struct Window* mainWindow;
    struct Window* threadDataWindow;
};

void prepareRunWindow(struct Window* runWindow, int threadCount);
void printRunWindow(struct systemContext* context, int x, int y);
void prepareMainScreen(struct Window* mainWindow, struct systemContext* context);
void printMainScreen(struct systemContext* context, int x, int y);
void appendOptionsMenuToWindow(struct Window* win, char **optionsArray, int optionCount, int doOptionsHaveIndexNo, struct UITextField *rawTextFieldsArray);
void appendBankerDataToWindow(struct Window* win, struct BankerData* bd, struct UITextField* textFieldArrayStart);
void appendThreadBehaviourDataToWindow(struct Window* win, struct threadTickBehaviour* tb, struct UITextField* textFieldArrayStart);
void printThreadDataScreen(struct systemContext* context, int x, int y);
void prepareThreadDataScreen(struct Window* win, struct systemContext* context);

int inputIntegerInTextField(struct UITextField* field,int initVal, int minVal, int maxVal, struct UITextField* errorMessage);
int navigateOptionsMenu(struct UITextField* optionsFieldArray, int optionCount);

#endif // __SYSTEMCONTEXTHELPER_C__