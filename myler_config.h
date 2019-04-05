#ifndef __MYLER_CONFIG_H__
#define __MYLER_CONFIG_H__

#include <stdbool.h>

#define MAX(a, b)     ((a) > (b) ? (a) : (b))

#define myler_stdout          stdout
#define myler_stdin           stdin
#define myler_stderr          stderr

// UI配置
#define UI_MAX_WIDTH          100
#define UI_MAX_HEIGHT         30
#define UI_COLOR              LYELLOW
#define UI_SEARCH_COLOR       LPURPLE
#define SEARCH_BUF_SIZE       (UI_MAX_WIDTH/3*2-2)
#define UI_TIMER_LENGTH       (UI_MAX_WIDTH-20)
#define UI_TITLE_COLOR        LYELLOW
#define UI_STATUS_COLOR       LGREEN
#define UI_MAIN_WINDOW_WITDH  (UI_MAX_WIDTH/3*2-2)
#define UI_MAIN_WINDOW_HEIGHT (UI_MAX_HEIGHT-8)

#define UI_TIMER_COLOR1       LAQUA
#define UI_TIMER_COLOR2       LBLUE
#define UI_TIMER_COLOR3       LGREEN

#define UI_LYRICS_COLOR1      LWHITE
#define UI_LYRICS_COLOR2      LAQUA

#define MAX_STR_BUF           256
#define MAX_TIME              0xffffffff
#define MAX_KEY_EVEVT         50

#define MAX_LIST_COUNT        100
#define MAX_LIST_SIZE         1000

#define LOCAL_LIST_PATH        "C:\\Users\\Administrator\\Desktop\\YylerPlayer"

#define DEFAULT_SPEED_UP_DOWN  10
#define DEFAULT_VOLUME_CHANGE  2
#define DEFAULT_VOLUME         5

#endif // !__MYLER_CONFIG_H__
