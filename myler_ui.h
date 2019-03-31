#ifndef __MYLER_UI_H__
#define __MYLER_UI_H__

#include "myler_config.h"
#include "myler_list.h"
#include "console.h"

typedef struct {
    char src_str[UI_WIDTH];
    char str[UI_WIDTH];
    char end_str[7];
    color_t color;
    int align_style;
    bool status;
} UIStringLine;

typedef struct {
    UIStringLine search_line;
    UIStringLine main_window_lines[UI_HEIGHT];
    UIStringLine list_lines[UI_HEIGHT];
    UIStringLine title_line;
    UIStringLine timer_line;
    UIStringLine bottom_line;
} UIStringBuffer;


typedef struct {
    int x, y, w, h;
    int disable;
} UIFrame;

typedef struct MylerUI {
    UIFrame list_frame;
    UIFrame main_window_frame;
    UIFrame search_frame;
    UIFrame timer_frame;

    UIStringBuffer buffer;

    unsigned int current_time, total_time;
    bool redraw;

    char message_buf[MAX_STR_BUF];
    bool message_status;
    int message_type;
} MylerUI;

typedef enum  {
    AlignLeft, AlignCenter, AlignRight,
} AlignStyle;

int  MylerUI_Init(MylerUI *ui);
void MylerUI_SetSearch(MylerUI *ui, int search_name, int search_type, const char *search_words);
void MylerUI_SetTimer(MylerUI *ui, unsigned int current_time, unsigned int total_time);
void MylerUI_SetListLine(MylerUI *ui, int line, color_t fcolor, int align_style, const char *format, ...);
void MylerUI_SetMainWindowLine(MylerUI *ui, int line, color_t fcolor, int align_style, const char *format, ...);
void MylerUI_SetTitle(MylerUI *ui, const char *title_format, ...);
void MylerUI_SetBottomLine(MylerUI *ui, int align_style, color_t color, const char *format, ...);
void MylerUI_SetListDisplay(MylerUI *ui, bool enable);
void MylerUI_SetMessage(MylerUI *ui, int type, const char *format, ...);
void MylerUI_ClearMainWindow(MylerUI *ui);
void MylerUI_Update(MylerUI *ui);
void MylerUI_SetListDisplay(MylerUI *ui, bool enable);
void MylerUI_SetMainWindowDisplay(MylerUI *ui, bool enable);
void MylerUI_SetSearchDisplay(MylerUI *ui, bool enable);
void MylerUI_SetTimerDisplay(MylerUI *ui, bool enable);
bool MylerUI_GetListDisable(MylerUI *ui);
bool MylerUI_GetMainWindowDisable(MylerUI *ui);
bool MylerUI_GetSearchDisable(MylerUI *ui);
bool MylerUI_GetTimerDisable(MylerUI *ui);
void MylerUI_Delet(MylerUI *ui, bool clear);

extern const char *search_name_string[];
extern const char *search_type_string[];

#endif
