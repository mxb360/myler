#ifndef __MYLER_LIST_H__
#define __MYLER_LIST_H__

#include "myler_lyrics.h"
#include "myler_config.h"
#include "music.h"

struct MylerList;

typedef struct MylerListNode {
    struct MylerListNode *prev;
    struct MylerListNode *next;
    struct MylerList *list;

    music_t *music;
    MylerLyrics *lyrics;

    char path[MAX_STR_BUF];
    char name[MAX_STR_BUF];
    char singer[MAX_STR_BUF];
    int  length;
} MylerListNode;

typedef struct MylerList {
    char list_name[MAX_STR_BUF];
    MylerListNode *current;
    MylerListNode *head;
    MylerListNode *tail;
    int music_count;
    int current_time;
    bool is_open;
    int play_status;
    bool is_play_end;
    bool replay;
    bool is_expand;
} MylerList;

typedef struct MylerListDisplayNode {
    struct MylerListDisplayNode *next;
    struct MylerListDisplayNode *prev;
    int type;
    MylerList *list;
    MylerListNode *music;
    const char *str1;
    const char *str2;
} MylerListDisplayNode;

typedef struct MylerListDisplay {
    MylerListDisplayNode *display_list;
    MylerListDisplayNode *head;
    MylerListDisplayNode *tail;
    MylerListDisplayNode *current_choice;
    MylerListDisplayNode *default_choice;
    int display_list_count;
} MylerListDisplay;

enum {
    IsListType, IsListNodeType,
};

#define PLAY_MODE_COUNT     4
enum PlayMode {
    PlayInOrder, PlayListLoop, PlayRepeatOne, PlayShuffle,
};


MylerList *MylerList_CreateList(const char *list_name);
MylerListNode *MylerList_AddLocalMusic(MylerList *list, const char *file_path);
MylerListNode *MylerList_SetCurrent(MylerList *list, int index);
int MylerList_Open(MylerList *list);
int MylerList_Play(MylerList *list);
int MylerList_Pause(MylerList *list);
int MylerList_Resume(MylerList *list);
int MylerList_SetNext(MylerList *list, int play_mode);
int MylerList_SetPrev(MylerList *list);
void MylerList_Delet(MylerList *list);
void MylerListDisplay_SetList(MylerListDisplay *list_display, MylerList *lists[], int list_count);
void MylerList_ClearCurrent(MylerList *list);
#endif
