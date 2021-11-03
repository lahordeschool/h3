#ifndef _H3_UI_H_
#define _H3_UI_H_

#include <h3/common.h>
H3_CAPI_BEGIN_BLOCK

bool H3_Ui_BeginWindow(const char* title);
void H3_Ui_EndWindow();
void H3_Ui_Printf(const char* fmt, ...);
bool H3_Ui_Button(const char* caption);
bool H3_Ui_Checkbox(const char* caption, bool* b);
bool H3_Ui_EditFloat(const char* caption, float* v, float min, float max);
bool H3_Ui_EditInt(const char* caption, int* v, int min, int max);

H3_CAPI_END_BLOCK
#endif /* _H3_UI_H_ */
