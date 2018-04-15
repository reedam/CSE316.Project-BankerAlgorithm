#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdarg.h>
#include"helper.h"
#include"UI.h"

void UIframe_init(struct UIframe *frame, int width, int height,
                    char lborder, char rborder, char roof, char floor, int textWrappingEnabled, int bordersEnabled)
{
    if(bordersEnabled)
    {
        // Reset internal width & height to compensate for border and roof
        width = width-2;
        height = height-2;
    }

    if(width<0||height<0||frame==NULL)return;
    int i;
    frame->screen = (char**)malloc(height * sizeof(char*));
    for(i=0;i<height;++i)frame->screen[i] = (char*)malloc(width * sizeof(char));
    frame->floor = floor;
    frame->roof = roof;
    frame->lborder = lborder;
    frame->rborder = rborder;
    frame->height = height;
    frame->width = width;
    frame->isTextWrappingEnabled = textWrappingEnabled;
    frame->bordersEnabled = bordersEnabled;
    frame->cursor[0]=0;
    frame->cursor[1]=0;
    frame->currentX = 0;
    frame->currentY = 0;
    UIframe_flush(frame);
}

void UIframe_flush(struct UIframe *frame)
{
    // Fill the screen with spaces.
    int i,j;
    for(i=0;i<frame->height;++i)
    {
        // Fill the row with spaces.
        memset(frame->screen[i], (int)' ', frame->width);
    }
    frame->cursor[0]=0;
    frame->cursor[1]=0;
}
void UIframe_nextLine(struct UIframe *frame)
{
    ++frame->cursor[0];
    frame->cursor[1]=0;
}

void UIframe_addLineChars(struct UIframe *frame, char *str, int startPos, int strLen, float position)
{
    // Increment cursor line.
    frame->cursor[0] = UIframe_setLineChars(frame,frame->cursor[0],str,startPos,strLen,position) + 1;
    frame->cursor[0] = frame->cursor[0]>frame->height-1?frame->height:frame->cursor[0];
    frame->cursor[1]=0;
}
void UIframe_addLine(struct UIframe *frame, char *str, int startPos, float position)
{
    UIframe_addLineChars(frame,  str, startPos, strlen(str)-startPos,position);
}
void UIframe_addLineAndBindTextFeilds(struct UIframe *frame, char *str, int startPos, float position, struct UITextField *textFieldArray)
{
    frame->cursor[0] = UIframe_setLineCharsAndBindTextFeilds(frame,frame->cursor[0],str,startPos,strlen(str),position,textFieldArray) + 1;
    frame->cursor[0] = frame->cursor[0]>frame->height-1?frame->height:frame->cursor[0];
    frame->cursor[1]=0;
}


int UIframe_internal_setLineChars(struct UIframe *frame, int line, char *str, int startPos, int strLen, float position, struct UITextField *textFieldArray, int textFieldArrayCounter)
{
    if(frame==NULL||str==NULL||line>=frame->height)return;
    position = clip(position, 0, 1);
    int printableStrLen = (strLen<=frame->width)?strLen:frame->width;
    int extraStrLen = strLen-printableStrLen;

    int bufferPos = (frame->width - printableStrLen)*position;
    for(;printableStrLen>0;++startPos,--printableStrLen)
    {
        if(str[startPos]=='\n')
        {
            extraStrLen+=printableStrLen;
            ++startPos;
            break;
        }
        else if(str[startPos]=='\t')
        {
            bufferPos+=4;
            continue;
        }
        else if(textFieldArray!=NULL&&str[startPos]=='%'&&str[startPos+1]=='f')
        {
            int fieldlen=0,tmp=0;
            char *p;
            p = &str[startPos+2];
            while(*p>'0'&&*p<'9')
            {
                fieldlen = fieldlen*10 + *p - '0';
                ++tmp;
                ++p;
            }
            fieldlen = clip(fieldlen,0,frame->width - startPos);
            textFieldArray[textFieldArrayCounter].charPos = bufferPos;
            textFieldArray[textFieldArrayCounter].length = fieldlen;
            textFieldArray[textFieldArrayCounter].linePos = line;
            textFieldArray[textFieldArrayCounter].parentFrame = frame;
            ++textFieldArrayCounter;
            bufferPos += fieldlen;
            startPos += 1+tmp;
            printableStrLen -= 1+tmp;
            continue;
        }
        frame->screen[line][bufferPos++] = str[startPos];
    }
    if(frame->isTextWrappingEnabled && extraStrLen>0)
        return UIframe_internal_setLineChars(frame,line+1,str,startPos,extraStrLen,0,textFieldArray,textFieldArrayCounter);
    return line;
}
int UIframe_setLineCharsAndBindTextFeilds(struct UIframe *frame, int line, char *str, int startPos, int strLen, float position, struct UITextField *textFieldArray)
{
    UIframe_internal_setLineChars(frame,line,str,startPos,strLen,position,textFieldArray,0);
}
int UIframe_setLineChars(struct UIframe *frame, int line, char *str, int startPos, int strLen, float position)
{
    return UIframe_internal_setLineChars(frame,line,str,startPos,strLen,position,NULL,0);
}
int UIframe_setLine(struct UIframe *frame, int line, char *str, int startPos, float position)
{
    return UIframe_setLineChars(frame,frame->cursor[0],str,startPos,strlen(str)-startPos,position);
}

/*
    This function prints the passed frame's buffer to the console at the passed X and Y positions.
    If the 'relative' parameter is passed 1, it prints the frame at x-y position relative to cursor's current position.
*/
void UIframe_print(struct UIframe *frame, int x, int y, int relative)
{
    ++y;
    frame->currentX = x;
    frame->currentY = y;
    int i,j;
    printf("%c[%d;%df",0x1b,y,x);
    if(frame->bordersEnabled)
    {
        for(i=0;i<frame->width+2;++i)printf("%c",frame->roof);
        ++y;
    }
    
    for(i=0;i<frame->height;++i)
    {
        printf("%c[%d;%df",0x1b,y,x);
        if(frame->bordersEnabled)printf("%c",frame->rborder);
        for(j=0;j<frame->width;++j)
        {
            printf("%c",frame->screen[i][j]);
        }
        if(frame->bordersEnabled)printf("%c",frame->lborder);
        ++y;
    }

    if(frame->bordersEnabled)
    {
        // Move cursor to next x-y pos.
        printf("%c[%d;%df",0x1b,y,x);
        // Print the floor.
        for(i=0;i<frame->width+2;++i)printf("%c",frame->floor);
        ++y;
    }
    // Print newline to flush stdout buffer.
    printf("\n");
}

/*-----------------------------------------------------*/
//              UITextField code
/*-----------------------------------------------------*/
/*

void UItextField_getFromFrame(struct UITextField* field, struct UIframe* frame, int linePos, int charPos, int length)
{
    if(frame==NULL||field==NULL||linePos>frame->height-1||charPos>frame->width-1)return;.
    clip(length,0,frame->width - charPos);
    field->linePos = linePos;
    field->charPos = charPos;
    field->parentFrame = frame;
    field->length = length;
}

void UItextField_internal_settext(struct UITextField* field, char *str, va_list lst)
{
    if(field==NULL)return;
    char *start = &field->parentFrame->screen[field->linePos][field->charPos];
    memset(start,' ',field->length);
    char postFeildchar = field->parentFrame->screen[field->linePos][field->charPos+field->length];
    vsnprintf(start,field->length+1,str,lst);
    field->parentFrame->screen[field->linePos][field->charPos+field->length] = postFeildchar;
}
void UItextField_setText(struct UITextField* field, char *str,...)
{
    va_list args;
    va_start(args,str);
    UItextField_internal_settext(field,str,args);
    va_end(args);
}
void UItextField_refresh(struct UITextField* field)
{
    printf("%c[%d;%df",0x1b,field->parentFrame->currentY+field->linePos,field->parentFrame->currentX+field->charPos+(field->parentFrame->bordersEnabled>0?1:0));
    char *start = &field->parentFrame->screen[field->linePos][field->charPos];
    printf("%.*s",field->length,start);
}
void UItextField_update(struct UITextField* field, char *str,...)
{
    va_list args;
    va_start(args,str);
    UItextField_internal_settext(field,str,args);
    va_end(args);
    UItextField_refresh(field);
}

void UItextField_setCursorAtFieldEnd(struct UITextField* field)
{
    printf("%c[%d;%df",0x1b,field->parentFrame->currentY+field->linePos,field->parentFrame->currentX+field->charPos+field->length+(field->parentFrame->bordersEnabled>0?1:0));
}

char* UItextField_getCharPointer(struct UITextField* field)
{
    return &(field->parentFrame->screen[field->linePos][field->charPos]);
}