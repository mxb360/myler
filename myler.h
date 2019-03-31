#ifndef __MYLER_H__
#define __MYLER_H__

#include "myler_config.h"
#include "myler_utils.h"
#include "myler_lyrics.h"
#include "myler_ui.h"
#include "myler_list.h"
#include "myler_cmdline.h"

#define DefaultList      1
#define TempList         0
#define LocalList        2

struct _MylerPlayer;

typedef void (*MylerKeyEventFunc)(struct _MylerPlayer *player);

typedef struct MylerKeyEvevt {
    int key;
    MylerKeyEventFunc key_event_func;
} MylerKeyEvevt;

typedef struct _MylerPlayer {
    MylerUI *ui;
    MylerCmdLine *cmdline;

    MylerList *list[MAX_LIST_COUNT];
    int list_count;
    MylerListDisplay list_display;
    MylerListDisplay search_display;

    char local_list_path[MAX_STR_BUF];

    MylerList *current_list;

    MylerKeyEvevt key_events[MAX_KEY_EVEVT];
    int key_event_count;
    bool show_main_lyric;

    int speed_up_down;
    int volume;
    int volume_change;
    int play_mode;
    int search_name;
    int search_type;
    char search_words[MAX_STR_BUF];

    char search_str_bak[UI_HEIGHT][MAX_STR_BUF];
} MylerPlayer;

int Myler_Init(MylerPlayer *player, int argc, char *argv[]);
void Myler_Update(MylerPlayer *player);
int Myler_CreateLocalList(MylerPlayer *player);
int Myler_CreateTempList(MylerPlayer *player);
int Myler_CreateDefaultList(MylerPlayer *player);
void Myler_AddKeyEvent(MylerPlayer *player, int key, MylerKeyEventFunc key_event_func);
void Myler_SearchLoaclMusic(MylerPlayer *player);
void Myler_Quit(MylerPlayer *player);

#endif
