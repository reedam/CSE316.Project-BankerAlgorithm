#ifndef __UI_C__
#define __UI_C__

struct UIframe{
    int width,height,currentX,currentY;
    int cursor[2];
    int isTextWrappingEnabled, bordersEnabled;
    char lborder,rborder,roof,floor;
    char **screen;
};
struct UITextField{
    struct UIframe *parentFrame;
    int length,linePos,charPos;
};

void UIframe_init(struct UIframe *frame, int width, int height,char lborder, char rborder, char roof, char floor, int textWrappingEnabled, int bordersEnabled);
void UIframe_addLine(struct UIframe *frame, char *str, int startPos, float position);
void UIframe_addLineAndBindTextFeilds(struct UIframe *frame, char *str, int startPos, float position, struct UITextField *textFieldArray);
void UIframe_addLineChars(struct UIframe *frame, char *str, int startPos, int strLen, float position);
int UIframe_setLine(struct UIframe *frame, int line, char *str, int startPos, float position);
int UIframe_setLineChars(struct UIframe *frame, int line, char *str, int startPos, int strLen, float position);
int UIframe_setLineCharsAndBindTextFeilds(struct UIframe *frame, int line, char *str, int startPos, int strLen, float position, struct UITextField *textFieldArray);
void UIframe_print(struct UIframe *frame, int x, int y, int relative);
void UIframe_flush(struct UIframe *frame);
void UIframe_nextLine(struct UIframe *frame);
void UItextField_getFromFrame(struct UITextField* field, struct UIframe* frame, int linePos, int charPos, int length);
void UItextField_setText(struct UITextField* field, char *str,...);
void UItextField_refresh(struct UITextField* field);
void UItextField_setCursorAtFieldEnd(struct UITextField* field);
char* UItextField_getCharPointer(struct UITextField* field);
void UItextField_update(struct UITextField* field, char *str,...);

#endif // __UI_C__