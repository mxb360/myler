#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "myler_ui.h"
#include "myler_utils.h"

const char *search_name_string[] = {"网易云音乐", "QQ音乐", "酷狗音乐", "本地音乐"};
const char *search_type_string[] = {"歌名", "歌单", "专辑", "歌词"};


/* 初始化MylerUI结构体（私有函数）
 */
static void DataReset(MylerUI *ui)
{
    memset(ui, 0, sizeof(MylerUI));

    ui->current_time = ui->total_time = MAX_TIME;

    ui->buffer.search_line.color = UI_SEARCH_COLOR;
    ui->buffer.status_line.color = UI_TITLE_COLOR;
    ui->main_window_frame.disable = true;
    ui->list_frame.disable = true;
    ui->search_frame.disable = true;
    ui->timer_frame.disable = true;
    ui->title_frame.disable = true;
}

/* （私有函数）
 *设置字符串的显示形式，如果字符串超出指定长度，则以“...”结尾，使总长度为length；
 * 否则按设置的对齐方式进行对齐，空格填充空白；该函数所得到的函数长度必为length。
 * string：待设置的字符串
 * length：字符串的允许长度
 * align_style：字符串的对齐方式 (左对齐:AlignLeft 居中对齐:AlignCenter 右对齐:AlignRight)
 * 返回：string本身
 */
static char *StringShowLimit(char *string, const char *end_string, int length, int align_style)
{
    int i = 0;
    char buf[UI_MAX_WIDTH];
    int end_string_len = strlen(end_string);

    if (length <= 0)
        return strcpy(string, "");

    // 先全部填充空格
    strcpy(buf, string);
    memset(string, ' ', length);                
    string[length] = 0;

    length -= end_string_len;
    while (buf[i] && i < length) {
        if (buf[i++] < 0)                           // 非ASCII字符
            i++;
        if (i == length - 2) {                      // 位置位于非ASCII字符内
            strcpy(buf + length - 4, " ...");
            i += 4;
            break;
        } else if (i == length - 3) {               // ASCII字符
            strcpy(buf + length - 3, "...");
            i += 3;
            break;
        }
    }

    int len = i; 

    // 按照所设置对齐方式移动字符串
    if (i == length - 2 || i == length - 3 || end_string_len) {
        strncpy(string, buf, len);
        strcpy(string + length, end_string);
    } else {
        length += end_string_len;

        if (align_style == AlignLeft)
            strncpy(string, buf, len);
        else if (align_style == AlignCenter)
            strncpy(string + (length - len) / 2, buf, len);
        else if (align_style == AlignRight)
            strncpy(string + length - len, buf, len);
    }
    return string;
}

/* 当界面发生变化时，此函数会被调用，用于更新界面上的文字排版（私有函数）
 */
static void BufferReload(UIStringLine *str_line, UIFrame *frame)
{
    strncpy(str_line->str, str_line->src_str, UI_MAX_WIDTH);
    StringShowLimit(str_line->str, str_line->end_str, frame->w - 2, str_line->align_style);
}

/* 显示列表界面（私有函数）
 */
static void ShowList(MylerUI *ui)
{
    if (ui->list_frame.disable)
        return;
    for (int i = 0; i < ui->list_frame.h - 2; i++) {
        if (ui->redraw || ui->buffer.list_lines[i].status) {
            BufferReload(&ui->buffer.list_lines[i], &ui->list_frame);
            con_set_pos(ui->list_frame.x + 1, ui->list_frame.y + i + 1);
            con_set_fcolor(ui->buffer.list_lines[i].color);
            fprintf(myler_stdout, "%s", ui->buffer.list_lines[i].str);
            ui->buffer.list_lines[i].status = false;
        }
    }
}

/* 显示主窗口界面（私有函数）
 */
static void ShowMainWindow(MylerUI *ui)
{
    if (ui->main_window_frame.disable)
        return;
    for (int i = 0; i < ui->main_window_frame.h - 2; i++) {
        if (ui->redraw || ui->buffer.main_window_lines[i].status) {
            BufferReload(&ui->buffer.main_window_lines[i], &ui->main_window_frame);
            con_set_pos(ui->main_window_frame.x + 1, ui->main_window_frame.y + i + 1);
            con_set_fcolor(ui->buffer.main_window_lines[i].color);
            fprintf(myler_stdout, "%s", ui->buffer.main_window_lines[i].str);
            ui->buffer.main_window_lines[i].status = false;
        }
    }
}

/* （私有函数）
 * 界面显示播放进度和播放时间 
 * current_time: 当前播放的时长（秒）
 * total_time: 总时长（秒）
 */
static void ShowTimer(MylerUI *ui)
{
    if (ui->timer_frame.disable)
        return;
    // 如果时间没变，直接返回
    if (ui->redraw || ui->buffer.timer_line.status) {
        ui->buffer.timer_line.src_str[0] = ' ';
        memset(ui->buffer.timer_line.src_str + 1, '-', UI_TIMER_LENGTH);

        int current = ui->total_time ? (int)(UI_TIMER_LENGTH * 1.0 * ui->current_time / ui->total_time) : 0;
        ui->buffer.timer_line.src_str[current + 1] = '*';
        sprintf(ui->buffer.timer_line.src_str + UI_TIMER_LENGTH + 1, " %02d:%02d/%02d:%02d",
                ui->current_time / 60, ui->current_time % 60, ui->total_time / 60, ui->total_time % 60);
        BufferReload(&ui->buffer.timer_line, &ui->timer_frame);

        con_set_fcolor(UI_TIMER_COLOR1);
        con_set_pos(ui->timer_frame.x + 1, ui->timer_frame.y + 2);
        // 绘制进度条
        bool is_color3_set = false;
        for (int i = 0; ui->buffer.timer_line.str[i]; i++)
        {
            fputc(ui->buffer.timer_line.str[i], myler_stdout);
            if (ui->buffer.timer_line.str[i] == '*')
                con_set_fcolor(UI_TIMER_COLOR2);
            if (i && !is_color3_set && ui->buffer.timer_line.str[i] == ' ') {
                con_set_fcolor(UI_TIMER_COLOR3);
                is_color3_set = true;
            }
        }
        ui->buffer.timer_line.status = false;
    }
}

/* 显示搜索栏（私有函数）
 */
static void ShowSearch(MylerUI *ui)
{
    if (ui->main_window_frame.disable || ui->search_frame.disable)
        return;
    if (ui->redraw || ui->buffer.search_line.status) {
        BufferReload(&ui->buffer.search_line, &ui->search_frame);
        con_set_pos(ui->search_frame.x + 1, ui->search_frame.y + 1);
        con_set_fcolor(ui->buffer.search_line.color);

        char *p;
        for (p = ui->buffer.search_line.str; *p != ' '; p++)
            fputc(*p, myler_stdout);
        con_reset_color();
        
        fprintf(myler_stdout, "%s", p);
        ui->buffer.search_line.status = false;
    }
}

static void ShowMessage(MylerUI *ui)
{
    static int t = 0;
    char buf[ui->max_width];

    if (ui->message_status) {
        if (ui->list_frame.disable && ui->main_window_frame.disable && ui->timer_frame.disable)
            con_set_pos(0, 0);
        else
            con_set_pos(0, ui->height);
        strcpy(buf, ui->message_buf);
        if (ui->message_type) {
            con_set_fcolor(LRED);
            StringShowLimit(buf, "", ui->max_width - 4, AlignLeft);
            fprintf(myler_stdout, "错误：%s", buf);
        } else {
            con_set_fcolor(LYELLOW);
            StringShowLimit(buf, "", ui->max_width, AlignLeft);
            fprintf(myler_stdout, "%s", buf);
        }
        t = 20;
        ui->message_status = false;
    }

    if (t) {
        t--;
        if (!t) {
            if (ui->list_frame.disable && ui->main_window_frame.disable && ui->timer_frame.disable)
                con_set_pos(0, 0);
            else
                con_set_pos(0, ui->height);
            strcpy(buf, " ");
            StringShowLimit(buf, "", ui->max_width, AlignLeft);
            fprintf(myler_stdout, "%s", buf);
        }
    }
}

/* 显示音乐状态栏（私有函数）
 */
static void ShowStatus(MylerUI *ui)
{
    if (ui->timer_frame.disable )
        return;
    if (ui->redraw || ui->buffer.status_line.status) {
        BufferReload(&ui->buffer.status_line, &ui->timer_frame);
        con_set_fcolor(UI_STATUS_COLOR);
        con_set_pos(ui->timer_frame.x + 1, ui->timer_frame.y + 1);
        fprintf(myler_stdout, "%s", ui->buffer.status_line.str);
        ui->buffer.status_line.status = false;
    }
}

static void ShowTitle(MylerUI *ui)
{
    if (ui->title_frame.disable )
        return;
    if (ui->redraw || ui->buffer.title_line.status) {
        BufferReload(&ui->buffer.title_line, &ui->title_frame);
        con_set_fcolor(UI_TITLE_COLOR);
        con_set_pos(ui->title_frame.x + 1, ui->title_frame.y + 1);
        fprintf(myler_stdout, "%s", ui->buffer.title_line.str);
        ui->buffer.title_line.status = false;
    }
}

static void ShowBottomLine(MylerUI *ui)
{
    if (ui->timer_frame.disable )
        return;
    if (ui->redraw || ui->buffer.bottom_line.status) {
        BufferReload(&ui->buffer.bottom_line, &ui->timer_frame);
        con_set_fcolor(ui->buffer.bottom_line.color);
        con_set_pos(ui->timer_frame.x + 1, ui->timer_frame.y + 3);
        fprintf(myler_stdout, "%s", ui->buffer.bottom_line.str);
        ui->buffer.bottom_line.status = false;
    }
}

/* 绘制矩形框（私有函数）
 */
static void DrawRect(int x, int y, int w, int h)
{
    for (int i = 0; i < h; i++) {
        //con_set_pos(x, y + i);
        for (int j = 0; j < w; j++)
        {
            if ((i == 0 && (j == 0 || j == w-1)) || (i == h-1 && (j == 0 || j == w-1)))
            {
                con_set_pos(x + j, y + i);
                fputc('+', myler_stdout);
            }
            else if (i == 0 || i == h-1)  {
                con_set_pos(x + j, y + i);
                fputc('-', myler_stdout);
            }
            else if (j == 0 || j == w-1) {
                con_set_pos(x + j, y + i);
                fputc('|', myler_stdout);
            }
        }
    }
}

/* （擦除界面私有函数）
 */
static void UIClear(MylerUI *ui)
{
    con_set_pos(0, 0);
    for (int i = 0; i <= ui->max_height; i++) {
        for (int j = 0; j < ui->max_width; j++)
            fputc(' ', myler_stdout);
        fputc('\n', myler_stdout);
    }
    con_set_pos(0, 0);
}

/* 绘制界面边框（私有函数）
 */
static void ShowUIFrame(MylerUI *ui)
{
    con_set_fcolor(UI_COLOR);
    if (!ui->title_frame.disable)
        DrawRect(ui->title_frame.x, ui->title_frame.y, ui->title_frame.w, ui->title_frame.h);
    if (!ui->timer_frame.disable)
        DrawRect(ui->timer_frame.x, ui->timer_frame.y, ui->timer_frame.w, ui->timer_frame.h);
    if (!ui->list_frame.disable)
        DrawRect(ui->list_frame.x, ui->list_frame.y, ui->list_frame.w, ui->list_frame.h);
    if (!ui->main_window_frame.disable && !ui->search_frame.disable)
        DrawRect(ui->search_frame.x, ui->search_frame.y, ui->search_frame.w, ui->search_frame.h);
    if (!ui->main_window_frame.disable)
        DrawRect(ui->main_window_frame.x, ui->main_window_frame.y, ui->main_window_frame.w, ui->main_window_frame.h);

    con_set_pos(0, ui->height);
    for (int i = ui->height; i < ui->max_height; i++) {
        for (int j = 0; j <= ui->width; j++)
            fputc(' ', myler_stdout);
        fputc('\n', myler_stdout);
    }
}


/* 初始化用户界面 
 */
int MylerUI_Init(MylerUI *ui)
{
    if (!ui)
        return -1;

    DataReset(ui);

    ui->max_height = UI_MAX_HEIGHT;
    ui->max_width = UI_MAX_WIDTH;

    // 初始化所有数据为默认值
    // 设置各个界面的默认显示状态
    MylerUI_SetTitleDisplay(ui, true);
    MylerUI_SetListDisplay(ui, true);
    MylerUI_SetMainWindowDisplay(ui, true);
    MylerUI_SetTimerDisplay(ui, true);
    MylerUI_SetSearchDisplay(ui, true);
    MylerUI_SetSearch(ui, 0, 0, "");
    MylerUI_SetTimer(ui, 0, 0);
    MylerUI_SetTitle(ui, "Myler Player 多功能音乐播放器 V2.0");

    // 设置界面缓冲区（只对Windows控制台有效）
    int w, h;

    con_get_buf_size(&w, &h);
    w = w > ui->max_width + 4 ? w : ui->max_width + 4;
    h = h > ui->max_height + 4 ? h : ui->max_height + 4;
    con_set_buf_size(w, h);
    //con_clear();
    //con_set_cur_visible(false);

    return 0;
}

/* 界面下显示搜索内容 
 * search_name: 搜索引擎（网易云、QQ、酷狗、本地）
 * search_type: 搜索类型（歌曲、歌单、专辑、歌词）
 * search_words: 搜索的关键字
 */
void MylerUI_SetSearch(MylerUI *ui, int search_name, int search_type, const char *search_words)
{
    Myler_Assert(ui);
    Myler_Assert(search_words);

    snprintf(ui->buffer.search_line.src_str, ui->max_width, "%s|%s|(搜索): %s",
             search_name_string[search_name], search_type_string[search_type], search_words);
    ui->buffer.search_line.status = true;
}

/* 设置主界面某行的显示内容
 * line: 待设置的行数
 * fcolor: 字体颜色
 * align_style: 字体对齐方式
 * format, ...: 待显示的字符串，支持printf型的格式化
 */
void MylerUI_SetMainWindowLine(MylerUI *ui, int line, color_t fcolor, int align_style, const char *format, ...)
{
    Myler_Assert(ui);
    Myler_Assert(line < ui->main_window_frame.h - 1);

    // 字符串格式化
    char buf[MAX_STR_BUF];
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, MAX_STR_BUF, format, ap);
    va_end(ap);

    if (!strcmp(buf, ui->buffer.main_window_lines[line].src_str))
        return;
    // 设置字符串显示格式
    strcpy(ui->buffer.main_window_lines[line].src_str, buf);
    ui->buffer.main_window_lines[line].status = true; 
    ui->buffer.main_window_lines[line].color = fcolor;
    ui->buffer.main_window_lines[line].align_style = align_style;
}


/* 设置列表界面某行的显示内容
 * line: 待设置的行数
 * fcolor: 字体颜色
 * align_style: 字体对齐方式
 * format, ...: 待显示的字符串，支持printf型的格式化
 */
void MylerUI_SetListLine(MylerUI *ui, int line, color_t fcolor, int align_style, const char *format, ...)
{
    Myler_Assert(ui);
    Myler_Assert(line < ui->list_frame.h - 1);

    // 字符串格式化
    va_list ap;
    char buf[MAX_STR_BUF];
    va_start(ap, format);
    vsnprintf(buf, MAX_STR_BUF, format, ap);
    va_end(ap);

    if (!strcmp(buf, ui->buffer.list_lines[line].src_str) && fcolor == ui->buffer.list_lines[line].color)
        return;
    // 设置字符串显示格式
    strcpy(ui->buffer.list_lines[line].src_str, buf);
    ui->buffer.list_lines[line].status = true;   
    ui->buffer.list_lines[line].color = fcolor;
    ui->buffer.list_lines[line].align_style = align_style;
    //sprintf(ui->buffer.list_lines[line].end_str, " %02d:%02d", 22, 45);
}

/* 设置位于进度条上方的标题文字 
 * title：标题字符串
 */
void MylerUI_SetStatusLine(MylerUI *ui, const char *title_format, ...)
{
    Myler_Assert(ui);
    Myler_Assert(title_format);

    char buf[MAX_STR_BUF];
    va_list ap;
    va_start(ap, title_format);
    vsprintf(buf, title_format, ap);
    va_end(ap);

    if (!strcmp(ui->buffer.status_line.src_str, buf))
        return;
    strncpy(ui->buffer.status_line.src_str, buf, ui->max_width);
    ui->buffer.status_line.status = true;
    ui->buffer.status_line.align_style = AlignCenter;
}

void MylerUI_SetTitle(MylerUI *ui, const char *title_format, ...)
{
    Myler_Assert(ui);
    Myler_Assert(title_format);

    char buf[MAX_STR_BUF];
    va_list ap;
    va_start(ap, title_format);
    vsprintf(buf, title_format, ap);
    va_end(ap);

    if (!strcmp(ui->buffer.title_line.src_str, buf))
        return;
    strncpy(ui->buffer.title_line.src_str, buf, ui->max_width);
    ui->buffer.title_line.status = true;
    ui->buffer.title_line.align_style = AlignCenter;
}

/* 设置底部栏文字 
 */
void MylerUI_SetBottomLine(MylerUI *ui, int align_style, color_t color, const char *format, ...)
{
    Myler_Assert(ui);
    Myler_Assert(format);

    char buf[MAX_STR_BUF];
    va_list ap;
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);

    if (!strcmp(ui->buffer.bottom_line.src_str, buf) && ui->buffer.bottom_line.color == color)
        return;
    strncpy(ui->buffer.bottom_line.src_str, buf, ui->max_width);
    ui->buffer.bottom_line.status = true;
    ui->buffer.bottom_line.color = color;
    ui->buffer.bottom_line.align_style = align_style;
}


/* 设置待显示的时间进度条和播放时间 
 * current_time: 当前播放的时长（秒）
 * total_time: 总时长（秒） 
 */
void MylerUI_SetTimer(MylerUI *ui,  unsigned int current_time, unsigned int total_time)
{
    Myler_Assert(ui);
    Myler_Assert(current_time <= total_time);

    if (ui->current_time == current_time && ui->total_time == total_time)
        return;
    ui->current_time = current_time;
    ui->total_time = total_time;
    ui->buffer.timer_line.status = true;
}

void MylerUI_SetMessage(MylerUI *ui,  int type, const char *format, ...)
{
    Myler_Assert(ui);
    Myler_Assert(format);

    char buf[MAX_STR_BUF];
    va_list ap;
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);

    strcpy(ui->message_buf, buf);
    ui->message_status = true;
    ui->message_type = type;
}

/* （稍后）从新绘制整个界面
 */
void MylerUI_Draw(MylerUI *ui)
{
    Myler_Assert(ui);

    ui->buffer.search_line.status = true;
    ui->buffer.status_line.status = true;
    ui->buffer.timer_line.status = true;
    ui->buffer.bottom_line.status = true;

    ui->redraw = true;
    
    for (int i = 0; i < ui->main_window_frame.h; i++) 
        ui->buffer.main_window_lines[i].status = true;
    for (int i = 0; i < ui->list_frame.h; i++) 
        ui->buffer.list_lines[i].status = true;
}

void MylerUI_ClearMainWindow(MylerUI *ui)
{
    for (int i = 0; i < ui->main_window_frame.h - 1; i++)
        MylerUI_SetMainWindowLine(ui, i, 0, AlignLeft, " ");
}



/* 更新UI
 */
void MylerUI_Update(MylerUI *ui)
{
    Myler_Assert(ui);

    if (ui->redraw) 
        ShowUIFrame(ui);

    ShowSearch(ui);
    ShowTimer(ui);
    ShowMainWindow(ui);
    ShowList(ui);
    ShowStatus(ui);
    ShowTitle(ui);
    ShowMessage(ui);
    ShowBottomLine(ui);

    ui->redraw = false;
}

void MylerUI_UpdateFrame(MylerUI *ui)
{
    ui->width = ui->max_width;
    ui->height = 0;
    ui->title_frame.x = ui->title_frame.y = 0;
    ui->list_frame.x = ui->list_frame.y = 0;
    ui->search_frame.x = ui->search_frame.y = 0;
    ui->main_window_frame.x = ui->main_window_frame.y = 0;
    ui->timer_frame.x = ui->timer_frame.y = 0;
    ui->title_frame.w = ui->title_frame.h = 0;
    ui->search_frame.w = ui->max_width, ui->search_frame.h = 0;
    ui->list_frame.w = ui->max_width, ui->list_frame.h = ui->max_height;
    ui->main_window_frame.w = ui->max_width, ui->main_window_frame.h = ui->max_height;
    ui->timer_frame.w = ui->timer_frame.h = 0;

    if (!ui->title_frame.disable) {
        ui->title_frame.w = ui->max_width;
        ui->title_frame.h = 3;
        ui->main_window_frame.y += 2;
        ui->list_frame.y += 2;
        ui->search_frame.y += 2;
        ui->timer_frame.y += 2;
        ui->height = ui->title_frame.y + ui->title_frame.h;
    }

    if (!ui->list_frame.disable) {
        ui->list_frame.w = ui->max_width;
        ui->list_frame.h = ui->max_height - ui->list_frame.y;
        ui->height = ui->list_frame.y + ui->list_frame.h;
    }

    if (!ui->search_frame.disable) {
        if (!ui->list_frame.disable) 
            ui->search_frame.w = ui->max_width / 3 * 2;
        ui->search_frame.h = 3;
        ui->main_window_frame.y += 2;    
    }

    if (!ui->main_window_frame.disable) {
        ui->main_window_frame.h = ui->max_height - ui->main_window_frame.y;
        if (!ui->list_frame.disable) {
            ui->main_window_frame.w = ui->max_width / 3 * 2;
            ui->list_frame.w = ui->max_width - ui->main_window_frame.w + 1;
            ui->main_window_frame.x = ui->list_frame.w - 1;
        }
        ui->search_frame.x = ui->main_window_frame.x;
        ui->height = ui->main_window_frame.y + ui->main_window_frame.h;
    }

    if (!ui->timer_frame.disable) {
        ui->timer_frame.w = ui->max_width;
        ui->timer_frame.h = 5;
        if (!ui->list_frame.disable) {
            ui->list_frame.h -= 4;
            ui->timer_frame.y = ui->list_frame.h + ui->list_frame.y - 1;
        }
        if (!ui->main_window_frame.disable) {
            ui->main_window_frame.h -= 4;
            ui->timer_frame.y = ui->main_window_frame.h + ui->main_window_frame.y - 1;
        }
        ui->height = ui->timer_frame.y + ui->timer_frame.h;
    }
}

/* 设置列表界面的显示状态
 * enable: 表示状态：显示(true)，不显示(false)
 */
void MylerUI_SetListDisplay(MylerUI *ui, bool enable)
{
    Myler_Assert(ui);
    ui->list_frame.disable = !enable;
    ui->list_frame.status = true;
    MylerUI_UpdateFrame(ui);
    MylerUI_Draw(ui);
}

bool MylerUI_GetListDisable(MylerUI *ui)
{
    return ui->list_frame.disable;
}

/* 设置主窗口界面的显示状态
 * enable: 表示状态：显示(true)，不显示(false)
 */
void MylerUI_SetMainWindowDisplay(MylerUI *ui, bool enable)
{
    Myler_Assert(ui);
    ui->main_window_frame.disable = !enable; 
    ui->main_window_frame.status = true;
    MylerUI_UpdateFrame(ui);
    MylerUI_Draw(ui);
}

bool MylerUI_GetMainWindowDisable(MylerUI *ui)
{
    return ui->main_window_frame.disable;
}

/* 设置搜索栏界面的显示状态
 * enable: 表示状态：显示(true)，不显示(false)
 */
void MylerUI_SetSearchDisplay(MylerUI *ui, bool enable)
{
    Myler_Assert(ui);
    if (ui->main_window_frame.disable) {
        ui->search_frame.disable = true;
        return;
    }

    ui->search_frame.disable = !enable;
    ui->search_frame.status = true;
    MylerUI_UpdateFrame(ui);
    MylerUI_Draw(ui);
}

bool MylerUI_GetSearchDisable(MylerUI *ui)
{
    return ui->search_frame.disable;
}

/* 设置时间和播放状态界面的显示状态
 * enable: 表示状态：显示(true)，不显示(false)
 */
void MylerUI_SetTimerDisplay(MylerUI *ui, bool enable)
{
    Myler_Assert(ui);
    ui->timer_frame.disable = !enable;
    ui->timer_frame.status = true;
    MylerUI_UpdateFrame(ui);
    MylerUI_Draw(ui);
}

bool MylerUI_GetTimerDisable(MylerUI *ui)
{
    return ui->timer_frame.disable;
}

void MylerUI_SetTitleDisplay(MylerUI *ui, bool enable)
{
    Myler_Assert(ui);
    ui->timer_frame.status = true;
    ui->title_frame.disable = !enable;
    MylerUI_UpdateFrame(ui);
    MylerUI_Draw(ui);
}

bool MylerUI_GetTitleDisable(MylerUI *ui)
{
    return ui->title_frame.disable;
}

void MylerUI_UpdateListChoice(UIStringLine lines[], int line_count)
{

}

/* 销毁界面
 * clear: 是否擦除界面
 */
void MylerUI_Delet(MylerUI *ui, bool clear)
{
    if (clear) {
        UIClear(ui);
        con_set_pos(0, ui->max_height + 1);
    }
    con_reset_color();
}
