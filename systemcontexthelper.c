#include"systemContextHelper.h"
#include"helper.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void prepareRunWindow(struct Window* runWindow, int threadCount)
{
    UIframe_flush(runWindow->frame);
    UIframe_addLine(runWindow->frame, "|Running System|", 0, 0.5);
    UIframe_addLine(runWindow->frame, "|~~~~~~~~~~~~~~|", 0, 0.5);
    UIframe_addLine(runWindow->frame,"",0,0);
    UIframe_addLine(runWindow->frame,"Below displayed is the current state of the threads that request resources from the banker.",0,0.15);
    UIframe_nextLine(runWindow->frame);
    UIframe_addLineAndBindTextFeilds(runWindow->frame,"Banker's Resources: %f15",0,0.15,runWindow->textFieldsArray);
    UIframe_addLine(runWindow->frame,"               CurrentTick | Requested |        RequestResult        | CurrentResources",0,0);

    int i;
    for(i=0;i<threadCount;++i)
    {
        // Add lines with text fields...
        struct UITextField* tfarray = &(runWindow->textFieldsArray[i*7+2]);
        UIframe_addLineAndBindTextFeilds(runWindow->frame,
            "    Thread %f2:    %f2/%f2       R%f2 x%f2     %f23       %f15",
            0,0,tfarray);
    }
    UIframe_nextLine(runWindow->frame);
    UIframe_nextLine(runWindow->frame);
    UIframe_addLineAndBindTextFeilds(runWindow->frame,"%f45",0,0.26,&runWindow->textFieldsArray[1]);
}

void printRunWindow(struct systemContext* context, int x, int y)
{
    int i,lasttick,k;
    char *lastResult;
    char curRes[30],tmp[10];
    struct threadTickBehaviour* currenttb;
    struct UITextField* tfarray;

    // Prepare the available banker's resources string.
    snprintf(curRes,29,"");
    for(k=0;k<context->bankerData->availableResourcesCount;++k)
    {
        snprintf(tmp,9," %d",context->bankerData->availableResourcesArray[k]);
        strcat(curRes,tmp);
    }
    UItextField_setText(context->runWindow->textFieldsArray,"%s",curRes);
    UItextField_refresh(context->runWindow->textFieldsArray);

    for(i=0;i<context->threadCount;++i)
    {
        currenttb = (context->tickBehaviourArray)+i;
        lasttick = currenttb->lastReqResponse==1?currenttb->currentTick-1:currenttb->currentTick;
        snprintf(curRes,29,"");
        switch(currenttb->lastReqResponse)
        {
            case  1:lastResult = "Acquired Resources"; break;
            case -1:lastResult = "Invalid ResourceIndex"; break;
            case -2:lastResult = "Invalid ProcessIndex"; break;
            case -3:lastResult = "Request more than max."; break;
            case -4:lastResult = "Unsafe State"; break;
            case -5:lastResult = "Resource Unavailable"; break;
            default: lastResult = "Unknown Result";
        }
        // Prepare the available resources string for this thread.
        for(k=0;k<currenttb->banker->availableResourcesCount;++k)
        {
            snprintf(tmp,9," %d",currenttb->banker->resourcesAllocatedMatrix[currenttb->processIndex][k]);
            strcat(curRes,tmp);
        }

        tfarray = &context->runWindow->textFieldsArray[i*7+2];
        // Set the text field values.
        UItextField_setText(tfarray+0,"%d",currenttb->processIndex);
        UItextField_setText(tfarray+1,"%d",lasttick+1);
        UItextField_setText(tfarray+2,"%d",currenttb->lifeTicks);
        UItextField_setText(tfarray+3,"%d",currenttb->perTickResourceDemandMatrix[lasttick][0]+1);
        UItextField_setText(tfarray+4,"%d",currenttb->perTickResourceDemandMatrix[lasttick][1]);
        UItextField_setText(tfarray+5,"%s",lastResult);
        UItextField_setText(tfarray+6,"%s",curRes);
        // Refresh all text fields of this thread.
        for(k=0;k<7;++k)UItextField_refresh(tfarray+k);
    }
    UIframe_print(context->runWindow->frame,x,y,0);
}
void appendThreadBehaviourDataToWindow(struct Window* win, struct threadTickBehaviour* tb, struct UITextField* textFieldArrayStart)
{
    struct UITextField* tfarray = textFieldArrayStart;
    int max,temp,i;
    // Chars taken by resource index. like R12
    temp = digitCount(tb->banker->availableResourcesCount)+1;
    for(i=0;i<tb->banker->availableResourcesCount;++i)
        // Max of all available resources.
        max = (max>tb->banker->maxResourcesArray[i])?max:tb->banker->maxResourcesArray[i];
    // Chars taken by max resource val.
    max = digitCount(max);
    // Get the max width of each text field.
    max = ((max>temp)?max:temp);
    // Chars. needed by tick count, like t12
    temp = digitCount(tb->lifeTicks)+1;
    // Larger of max or strlen(tickChars) like t12
    max = max>temp?max:temp;
    // Max now stores the width of each text field.
    // +1 to store chars like R for resource and +/- for count.
    max += 1;
    // We now get the digitcount of this max to specify as field size.
    temp = 3+digitCount(max);
    char *tmpstr = malloc((temp+1)*sizeof(char));
    snprintf(tmpstr,temp+1," %%f%d",max);
    // Claculate the max no. of such fields per line of the frame.
    max = (win->frame->width-10)/max;
    // Make string to store generation line.
    char *str = malloc(((temp*max)+1+1)*sizeof(char));
    char *str1 = malloc(((temp*max)+10+1+1)*sizeof(char));
    struct UITextField* tickfields = tfarray;
    struct UITextField* resfields = tfarray+tb->lifeTicks;
    struct UITextField* resCfields = resfields+tb->lifeTicks;
    i=0;
    while(i<tb->lifeTicks)
    {
        strcpy(str,"");
        for(temp=0;temp<max&&i<tb->lifeTicks;++i,++temp)
        {
            strcat(str,tmpstr);
        }
        strcpy(str1,"\tTick No.:");
        strcat(str1,str);
        UIframe_addLineAndBindTextFeilds(win->frame,str1,0,0,tickfields);
        tickfields += temp;

        strcpy(str1,"\tResource:");
        strcat(str1,str);
        UIframe_addLineAndBindTextFeilds(win->frame,str1,0,0,resfields);
        resfields += temp;
        
        strcpy(str1,"\t   Count:");
        strcat(str1,str);
        UIframe_addLineAndBindTextFeilds(win->frame,str1,0,0,resCfields);
        resCfields += temp;
    }
    free(tmpstr);
    free(str);
    free(str1);
}
void prepareThreadDataScreen(struct Window* win, struct systemContext* context)
{
    struct UIframe* frame = win->frame;
    UIframe_addLine(frame,"Thread Behaviour",0,0.5);
    UIframe_addLine(frame,"Here you can view the thread behaviour for the current system configuration.",0,0.5);
    UIframe_addLine(frame,"-ve resource count means free the resource.",0,0.5);
    char *str = malloc((8+digitCount(context->threadCount)+1)*sizeof(char));
    int i;
    struct UITextField* tfarray = context->threadDataWindow->textFieldsArray;
    for(i=0;i<context->threadCount;++i)
    {
        UIframe_nextLine(frame);
        sprintf(str,"Thread %d",i);
        UIframe_addLine(win->frame,str,0,0);
        appendThreadBehaviourDataToWindow(win,&context->tickBehaviourArray[i],tfarray);
        tfarray += 3*context->tickBehaviourArray[i].lifeTicks;
    }
    UIframe_nextLine(context->threadDataWindow->frame);
    UIframe_addLineAndBindTextFeilds(context->threadDataWindow->frame,"%f8\t%f8",0,0.2,tfarray);
    free(str);
}
void printThreadDataScreen(struct systemContext* context, int x, int y)
{
    int i,t;
    struct UITextField* tfarray = context->threadDataWindow->textFieldsArray;
    for(i=0;i<context->threadCount;++i)
    {
        for(t=0;t<context->tickBehaviourArray[i].lifeTicks;++t)
        {
            UItextField_setText(tfarray,"t%.*d",tfarray->length-1,t+1);
            UItextField_setText(tfarray+context->tickBehaviourArray[i].lifeTicks,"r%.*d",tfarray->length-1,context->tickBehaviourArray[i].perTickResourceDemandMatrix[t][0]);
            UItextField_setText(tfarray+(2*context->tickBehaviourArray[i].lifeTicks),"%*d",tfarray->length,context->tickBehaviourArray[i].perTickResourceDemandMatrix[t][1]);
            ++tfarray;
        }
        tfarray += 2*context->tickBehaviourArray[i].lifeTicks;
    }
    //UItextField_setText(tfarray++,"-Edit");
    UItextField_setText(tfarray,"-Back");
    UIframe_print(context->threadDataWindow->frame,x,y,0);
    navigateOptionsMenu(tfarray,1);
}
void appendBankerDataToWindow(struct Window* win, struct BankerData* bd, struct UITextField* textFieldArrayStart)
{
    int i,t,max=0;
    struct UITextField* tfarray = textFieldArrayStart;
    for(t=0;t<bd->availableResourcesCount;++t)
        // Max of all available resources.
        max = (max>bd->maxResourcesArray[t])?max:bd->maxResourcesArray[t];
    // Chars. taken by max resource val.
    max = digitCount(max);
    int tmp = digitCount(max);
    char *tmpstr = malloc((tmp+4)*sizeof(char));
    char *str = malloc(((tmp+4)*bd->availableResourcesCount+50)*sizeof(char));

    sprintf(str,"\tMax Available Resources Array:[ ");
    for(t=0;t<bd->availableResourcesCount;++t)
    {
        snprintf(tmpstr,tmp+4,"%cf%d ",'%',max);
        strcat(str,tmpstr);
    }
    strcat(str,"]");
    UIframe_addLineAndBindTextFeilds(win->frame,str,0,0,tfarray);
    tfarray += bd->availableResourcesCount;

    UIframe_addLine(win->frame,"\tMax Resource Demand Matrix:-",0,0);
    // Append the Max Resource Demand Matrix and its text fields to the frame.
    for(i=0;i<bd->processCount;++i)
    {
        snprintf(str,(max+1)*bd->availableResourcesCount,"\t\tP%d: ",i);
        for(t=0;t<bd->availableResourcesCount;++t)
        {
            snprintf(tmpstr,tmp+4,"%cf%d ",'%',max);
            strcat(str,tmpstr);
        }
        UIframe_addLineAndBindTextFeilds(win->frame,str,0,0,tfarray);
        tfarray += bd->availableResourcesCount;
    }
    // Append the Allocated Resources Matrix and its text fields to the frame.
    UIframe_addLine(win->frame,"\tResources Allocated Matrix:-",0,0);
    for(i=0;i<bd->processCount;++i)
    {
        snprintf(str,(max+1)*bd->availableResourcesCount,"\t\tP%d: ",i);
        for(t=0;t<bd->availableResourcesCount;++t)
        {
            snprintf(tmpstr,tmp+4,"%cf%d ",'%',max);
            strcat(str,tmpstr);
        }
        UIframe_addLineAndBindTextFeilds(win->frame,str,0,0,tfarray);
        tfarray += bd->availableResourcesCount;
    }
    free(tmpstr);
    free(str);
}

void prepareMainScreen(struct Window* mainWindow, struct systemContext* context)
{
    UIframe_flush(mainWindow->frame);
    UIframe_addLine(mainWindow->frame,"Welcome",0,0.5);
    UIframe_addLine(mainWindow->frame,"~~~~~~~",0,0.5);
    UIframe_nextLine(mainWindow->frame);
    UIframe_addLine(mainWindow->frame,"This program simulates the banker's algorithm by making threads that request resources from a banker.",0,0.1);
    UIframe_addLine(mainWindow->frame,"Its current configuration is as follows:-",0,0);

    appendBankerDataToWindow(mainWindow,context->bankerData,mainWindow->textFieldsArray);
}
void printMainScreen(struct systemContext* context, int x, int y)
{
    int i,t;
    struct UITextField* tfarray = context->mainWindow->textFieldsArray;
    for(t=0;t<context->bankerData->availableResourcesCount;++t)
    {
        UItextField_setText(tfarray++,"%d",context->bankerData->availableResourcesArray[t]);
    }
    for(i=0;i<context->bankerData->processCount;++i)
    {
        for(t=0;t<context->bankerData->availableResourcesCount;++t)
        {
            UItextField_setText(tfarray++,"%d",context->bankerData->resourcesDemandMatrix[i][t]);
        }
    }
    for(i=0;i<context->bankerData->processCount;++i)
    {
        for(t=0;t<context->bankerData->availableResourcesCount;++t)
        {
            UItextField_setText(tfarray++,"%d",context->bankerData->resourcesAllocatedMatrix[i][t]);
        }
    }
    UIframe_print(context->mainWindow->frame,x,y,0);
}
void appendOptionsMenuToWindow(struct Window* win, char **optionsArray, int optionCount, int doOptionsHaveIndexNo, struct UITextField *rawTextFieldsArray)
{
    int max=0,i,sz;
    for(i=0;i<optionCount;++i)
    {
        sz = strlen(optionsArray[i]);
        max = (max>sz?max:sz);
    }
    max = max+2+digitCount(optionCount)+1;
    sz = digitCount(max);
    char *str = malloc((sz+4)*sizeof(char));
    sprintf(str,"\t%%f%d",max);
    for(i=0;i<optionCount;++i)
    {
        UIframe_addLineAndBindTextFeilds(win->frame,str,0,0,&rawTextFieldsArray[i]);
        if(doOptionsHaveIndexNo)UItextField_setText(&rawTextFieldsArray[i],"%d| %s",i+1,optionsArray[i]);
        else UItextField_setText(&rawTextFieldsArray[i]," | %s",optionsArray[i]);
    }
    free(str);
}

int navigateOptionsMenu(struct UITextField* optionsFieldArray, int optionCount)
{
    char **p = malloc(optionCount*sizeof(char*));
    char pointer = '>',prev;
    int i;
    char in;
    for(i=0;i<optionCount;++i)
    {
        p[i] = UItextField_getCharPointer(&optionsFieldArray[i]);
        UItextField_refresh(&optionsFieldArray[i]);
    }
    i=0;
    // set pointer to first entry.
    prev = *p[i];
    *p[i] = pointer;
    UItextField_refresh(&optionsFieldArray[i]);
    while(1)
    {
        in = getch();
        // Break on enter key.
        if(in=='\n')break;
        // Check for arrow keys.
        else if(in == '\033')
        {
            // Skip the '[' char which comes with arrow keys.
            getch();
            switch(getch())
            {
                case 'A':
                    // code for arrow up
                    // Skip if at topmost entry.
                    if(i==0)continue;
                    --i;
                    *p[i+1] = prev;
                    prev = *p[i];
                    *p[i] = pointer;
                    UItextField_refresh(&optionsFieldArray[i+1]);
                    UItextField_refresh(&optionsFieldArray[i]);
                    break;
                case 'B':
                    // code for arrow down
                    // Skip if at lowest entry.
                    if(i==optionCount-1)continue;
                    ++i;
                    *p[i-1] = prev;
                    prev = *p[i];
                    *p[i] = pointer;
                    UItextField_refresh(&optionsFieldArray[i-1]);
                    UItextField_refresh(&optionsFieldArray[i]);
                    break;
                case 'C':
                    // code for arrow right
                    break;
                case 'D':
                    // code for arrow left
                    break;
            }
            // set pointer to current entry.
        }
    }
    // Restore text field.
    *p[i] = prev;
    free(p);
    return i;
}
/*
 *  =================================================================
 *  |                   Input Functions                             |
 *  =================================================================
*/
int inputIntegerInTextField(struct UITextField* field,int initVal, int minVal, int maxVal, struct UITextField* errorMessage)
{
    UItextField_setText(field,"%d",initVal);
    UItextField_refresh(field);
    // Move the cursor to the text field.
    UItextField_setCursorAtFieldEnd(field);

    char in;
    int val=initVal,curpos=field->length;
    while(1)
    {
        in=getch();
        // Break on enter key.
        if(in=='\n')break;
        // Errase on backspace key.
        else if(in == 127 && curpos)
        {
            val /= 10;
            putchar('\b');putchar('\0');putchar('\b');
            --curpos;
        }
        else if(in>='0'&&in<='9' && curpos<field->length)
        {
            val*=10;
            val += in-'0';
            if(val<minVal||val>maxVal)
            {
                val/=10;
                UItextField_setText(errorMessage,"Max. value=%d; Min. value=%d",maxVal,minVal);
                UItextField_refresh(errorMessage);
                UItextField_setCursorAtFieldEnd(field);
                curpos = field->length;
            }
            else 
            {
                putchar(in);
                ++curpos;
            }
        }
    }
    return val;
}