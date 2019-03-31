#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "myler.h"

/* 按键触发：反转列表界面显 示状态（私有函数） */
static void ToggleListDisplay(MylerPlayer *player)
{
    MylerUI_SetListDisplay(player->ui, MylerUI_GetListDisable(player->ui));
}

/* 按键触发：反转主窗口界面显示状态（私有函数） */
static void ToggleMainWindowDisplay(MylerPlayer *player)
{
    MylerUI_SetMainWindowDisplay(player->ui, MylerUI_GetMainWindowDisable(player->ui));
    player->show_main_lyric = !MylerUI_GetMainWindowDisable(player->ui);
}
 
/* 按键触发：反转搜索栏显示状态（私有函数） */
static void ToggleSearchDisplay(MylerPlayer *player)
{
    MylerUI_SetSearchDisplay(player->ui, MylerUI_GetSearchDisable(player->ui));
}

/* 按键触发：反转时间和状态界面显示状态（私有函数） */
static void ToggleTimerDisplay(MylerPlayer *player)
{
    MylerUI_SetTimerDisplay(player->ui, MylerUI_GetTimerDisable(player->ui));
}

/* 按键触发：暂停/继续播放音乐（私有函数） */
static void PlayOrPause(MylerPlayer *player)
{
    if (player->current_list->play_status == MUSIC_PAUSED)
        MylerList_Resume(player->current_list);
    else if (player->current_list->play_status == MUSIC_PLAYING)
        MylerList_Pause(player->current_list);
}

/* 按键触发：下一曲（私有函数） */
static void Next(MylerPlayer *player)
{
    if (player->current_list) {
        if (player->play_mode == PlayRepeatOne)
            MylerList_SetNext(player->current_list, PlayInOrder);
        else
            MylerList_SetNext(player->current_list, player->play_mode);
        MylerUI_SetMessage(player->ui, 0, "下一曲");
    }
}

/* 按键触发：上一曲（私有函数） */
static void Prev(MylerPlayer *player)
{
    if (player->current_list) {
        MylerList_SetPrev(player->current_list);
        MylerUI_SetMessage(player->ui, 0, "上一曲");
        player->current_list->is_play_end = false;
    }
}

static void Replay(MylerPlayer *player)
{
    if (player->current_list) {
        MylerList_ClearCurrent(player->current_list);
        player->current_list->replay = true;
        MylerUI_SetMessage(player->ui, 0, "重新播放");
    }
}

/* 按键触发：快进（私有函数） */
static void SpeedUp(MylerPlayer *player)
{
    if (player->current_list && player->current_list->current) {
        int t = player->current_list->current_time + player->speed_up_down * 1000;
        if (player->current_list->is_open) {
            if (t < player->current_list->current->length)
                music_play(player->current_list->current->music, t, 0, 0);
            else
                music_play(player->current_list->current->music,
                           player->current_list->current->length, 0, 0);
            MylerUI_SetMessage(player->ui, 0, "快进%d秒", player->speed_up_down);
        }
    }
}

/* 按键触发：快退（私有函数） */
static void SpeedDown(MylerPlayer *player)
{
    if (player->current_list && player->current_list->current) {  
        if (player->current_list->is_open) {
            int t = player->current_list->current_time - player->speed_up_down * 1000;
            if (t > 0)
                music_play(player->current_list->current->music, t, 0, 0);
            else
                music_play(player->current_list->current->music, 0, 0, 0);
            MylerUI_SetMessage(player->ui, 0, "快退%d秒", player->speed_up_down);
        }
    }
}

/* 按键触发：音量+（私有函数） */
static void VolumeUp(MylerPlayer *player)
{
    player->volume += player->volume_change;
    if (player->volume > 100)
        player->volume = 100;
    MylerUI_SetMessage(player->ui, 0, "音量：%d", player->volume);
    if (player->current_list->is_open)
        music_set_volume(player->current_list->current->music, player->volume * 10);
}

/* 按键触发：音量-（私有函数） */
static void VolumeDown(MylerPlayer *player)
{
    player->volume -= player->volume_change;
    if (player->volume < 0)
        player->volume = 0;
    MylerUI_SetMessage(player->ui, 0, "音量：%d", player->volume);
    if (player->current_list->is_open)
        music_set_volume(player->current_list->current->music, player->volume * 10);
}

static void MoveUp(MylerPlayer *player)
{
    MylerListDisplay *display = &player->list_display;
    if (display->current_choice->prev) {
        display->current_choice = display->current_choice->prev;
        if (display->current_choice == display->head->prev)
            display->head = display->head->prev;
    }
}

static void MoveDown(MylerPlayer *player)
{
    MylerListDisplay *display = &player->list_display;
    if (display->current_choice->next) {
        display->current_choice = display->current_choice->next;
        if (display->current_choice == display->tail->next)
            display->head = display->head->next;
    }
}

static void PlayChoice(MylerPlayer *player)
{
    if (player->list_display.current_choice->type == IsListNodeType) {
        MylerList_ClearCurrent(player->current_list);
        player->current_list = player->list_display.current_choice->list;
        player->current_list->current = player->list_display.current_choice->music;
        player->current_list->is_play_end = false;
        player->current_list->replay = true;
        MylerUI_SetMessage(player->ui, 0, "开始播放“%s - %s”", player->current_list->current->name, player->current_list->current->singer);
    }
}

static const char *_mode_str[] = {
    "顺序播放", "列表循环", "单曲循环", "随机播放",
};

static void SetPlayMode(MylerPlayer *player)
{
    player->play_mode = (player->play_mode + 1) % PLAY_MODE_COUNT;
    MylerUI_SetMessage(player->ui, 0, "播放模式：%s", _mode_str[player->play_mode]);
}

static void Search(MylerPlayer *player)
{
    player->show_main_lyric = false;
    MylerUI_ClearMainWindow(player->ui);
    MylerUI_SetMainWindowLine(player->ui, 0, LWHITE, AlignLeft, "%s搜索%s: “%s”中，请稍后...",
                              search_name_string[player->search_name],
                              search_type_string[player->search_type], player->search_words);
    Myler_SearchLoaclMusic(player);
}

static void ShowLyrics(MylerPlayer *player)
{
    player->show_main_lyric = true;

}

static void ShowSearch(MylerPlayer *player)
{
    player->show_main_lyric = false;
}

static int Myler_AddLoaclFileToList(MylerList *list, const char *file_path)
{
    char *files[MAX_LIST_SIZE];
    char buf[MAX_STR_BUF + 10];
    int n;

    strcpy(buf, file_path);
    if ((n = GetFiles(files, MAX_LIST_SIZE, buf)) < 0) 
        return -1; 

    for (int i = 0; i < n; i++) {
        if (!strcmp(files[i] + strlen(files[i]) - strlen(".mp3"), ".mp3"))
            MylerList_AddLocalMusic(list, files[i]);
    }
    list->is_expand = true;
    return 0;
}

void Myler_ExecCmdLine(MylerPlayer *player)
{
    if (player->cmdline->have_help)  {
        MylerCmdLine_PrintHelp();
        Myler_Quit(0);
    }

    for (int i = 0; i < player->cmdline->music_name_count; i++) {
        Myler_AddLoaclFileToList(player->list[TempList], player->cmdline->music_name[i]);
    }
}


/* 播放器初始化 */
int Myler_Init(MylerPlayer *player, int argc, char *argv[])
{
    Myler_Assert(player);

    srand((unsigned int)time(NULL));

    player->current_list = NULL;
    player->list_count = 0;
    player->key_event_count = 0;
    player->show_main_lyric = true;

    player->speed_up_down = DEFAULT_SPEED_UP_DOWN;
    player->volume_change = DEFAULT_VOLUME_CHANGE;
    player->volume = DEFAULT_VOLUME;
    player->play_mode = PlayInOrder;

    player->search_name = 1;
    player->search_type = 0;
    strcpy(player->search_words, "出山");

    player->cmdline = malloc(sizeof(MylerCmdLine));
    player->ui = malloc(sizeof(MylerUI));

    int err_code;
    if ((err_code = MylerCmdLine_Init(player->cmdline, argc, argv)))
        return err_code;

    // 创建本地列表
    Myler_CreateDefaultList(player);
    Myler_CreateTempList(player);
    strncpy(player->local_list_path, LOCAL_LIST_PATH, MAX_STR_BUF);
    if (Myler_CreateLocalList(player)) {
        Myler_PrintError("本地列表创建失败");
        return -1;
    }

    Myler_ExecCmdLine(player);
    if (MylerUI_Init(player->ui))
        return -1;

    MylerUI_SetSearch(player->ui, player->search_name, player->search_type, player->search_words);

    // 默认的按键事件绑定
    Myler_AddKeyEvent(player, 'K', ToggleListDisplay);
    Myler_AddKeyEvent(player, 'Q', Myler_Quit);
    Myler_AddKeyEvent(player, 'J', ToggleMainWindowDisplay);
    Myler_AddKeyEvent(player, 'H', ToggleTimerDisplay);
    Myler_AddKeyEvent(player, 'L', ToggleSearchDisplay);
    Myler_AddKeyEvent(player, '.', Next);
    Myler_AddKeyEvent(player, ' ', PlayOrPause);
    Myler_AddKeyEvent(player, ',', Prev);
    Myler_AddKeyEvent(player, 'A', SpeedDown);
    Myler_AddKeyEvent(player, 'D', SpeedUp);
    Myler_AddKeyEvent(player, 'W', VolumeUp);
    Myler_AddKeyEvent(player, 'S', VolumeDown);
    Myler_AddKeyEvent(player, KEY_UP, MoveUp);
    Myler_AddKeyEvent(player, KEY_DOWN, MoveDown);
    Myler_AddKeyEvent(player, KEY_ENTER, PlayChoice);
    Myler_AddKeyEvent(player, 'M', SetPlayMode);
    Myler_AddKeyEvent(player, 'R', Replay);
    Myler_AddKeyEvent(player, 'Y', ShowSearch);
    Myler_AddKeyEvent(player, 'U', ShowLyrics);
    Myler_AddKeyEvent(player, 'E', Search);

    player->current_list = player->list[LocalList];
    MylerList_SetCurrent(player->list[LocalList], 0);
    MylerListDisplay_SetList(&player->list_display, player->list, player->list_count);

    return 0;
}

/* 为播放器添加按键绑定 */
void Myler_AddKeyEvent(MylerPlayer *player, int key, MylerKeyEventFunc key_event_func)
{
    Myler_Assert(player);
    Myler_Assert(key_event_func);

    player->key_events[player->key_event_count++] = (MylerKeyEvevt){key, key_event_func};
}

int Myler_CreateDefaultList(MylerPlayer *player)
{
    player->list[DefaultList] = MylerList_CreateList("默认列表");
    if (!player->list[DefaultList])
        return -1;

    player->list_count++;
    return 0;
}

int Myler_CreateTempList(MylerPlayer *player)
{
    player->list[TempList] = MylerList_CreateList("临时列表");
    if (!player->list[TempList])
        return -1;

    player->list_count++;
    return 0;
}

int Myler_CreateLocalList(MylerPlayer *player)
{
    player->list[LocalList] = MylerList_CreateList("本地列表");
    if (!player->list[LocalList])
        return -1;
    char *files[MAX_LIST_SIZE];
    char buf[MAX_STR_BUF + 10];
    int n;

    strcpy(buf, player->local_list_path);
    strcat(buf, "\\*.mp3");
    if ((n = GetFiles(files, MAX_LIST_SIZE, buf)) < 0) {
        sprintf(buf, "mkdir %s", player->local_list_path);
        if (system(buf)) {
            return -1; 
        }

        strcpy(buf, player->local_list_path);
        strcat(buf, "\\*.mp3");
        if ((n = GetFiles(files, MAX_LIST_SIZE, buf)) < 0) {
            return -1;
        }
    }

    for (int i = 0; i < n; i++)
        MylerList_AddLocalMusic(player->list[LocalList], files[i]);
    player->list[LocalList]->is_expand = true;
    player->list_count++;
    return 0;
} 

void Myler_SearchLoaclMusic(MylerPlayer *player)
{

}

void Myler_ListDisplayUpdate(MylerPlayer *player, MylerListDisplay *list_display, int line_count)
{
    int line = 0;
    MylerListDisplayNode *node = list_display->head;
    while (node && line < line_count)
    {
        if (node->type == IsListType && node->list->is_expand) {
            if (list_display->current_choice != node)
                MylerUI_SetListLine(player->ui, line, LBLUE, AlignLeft, "v %s", node->str1);
            else
                MylerUI_SetListLine(player->ui, line, GREEN, AlignLeft, "◆%s", node->str1);
        } else if (node->type == IsListType && !node->list->is_expand) {
            if (list_display->current_choice != node)
                MylerUI_SetListLine(player->ui, line, LBLUE, AlignLeft, "> %s", node->str1);
            else
                MylerUI_SetListLine(player->ui, line, GREEN, AlignLeft, "◆%s", node->str1);
        } else {
            if (list_display->current_choice == node)   
                MylerUI_SetListLine(player->ui, line, LGREEN, AlignLeft, "◆* %s - %s", node->str1, node->str2);
            else if (node->music == player->current_list->current)
                MylerUI_SetListLine(player->ui, line, GREEN, AlignLeft, "  * %s - %s", node->str1, node->str2);
            else
                MylerUI_SetListLine(player->ui, line, LWHITE, AlignLeft, "  * %s - %s", node->str1, node->str2);
        }
        list_display->tail = node;
        node = node->next;
        line++;
    }   
    
    while (line < line_count - 1) {
        MylerUI_SetListLine(player->ui, line, WHITE, AlignLeft, "  ");
        line++;
    }
}

void Myler_Update(MylerPlayer *player)
{
    // 按键事件
    while (con_have_key()) {
        int key = con_get_key(1);
        for (int i = 0; i < player->key_event_count; i++)
            if (key == player->key_events[i].key)
                player->key_events[i].key_event_func(player);
    }

    // 播放
    if (player->current_list->replay) {
        MylerList_Open(player->current_list);
        if (MylerList_Play(player->current_list) == -2) {
            con_sleep(500);
            MylerUI_SetMessage(player->ui, 1, "音乐播放失败：%s", music_get_last_error());
            con_sleep(500);
            MylerList_SetNext(player->current_list, player->play_mode);
            return;
        }
        music_set_volume(player->current_list->current->music, player->volume * 10);
        player->current_list->replay = false;
    }

    // 进度和时间
    if (player->current_list->is_open) {
        player->current_list->current_time = music_get_current_length(player->current_list->current->music);
        player->current_list->play_status = music_get_status(player->current_list->current->music);
        MylerUI_SetTimer(player->ui, player->current_list->current_time / 1000, player->current_list->current->length / 1000);
        if (player->current_list->current_time >=  player->current_list->current->length) {
            MylerList_SetNext(player->current_list, player->play_mode);
            return;
        }
    }

    // 状态栏
    if (player->current_list->is_play_end) {
        MylerUI_SetTitle(player->ui, "播放器空闲中...");
        MylerUI_SetTimer(player->ui, 0, 0);
        MylerUI_ClearMainWindow(player->ui);
    } else if (player->current_list->play_status == MUSIC_PLAYING)
        MylerUI_SetTitle(player->ui, "正在播放“%s - %s”", player->current_list->current->name,
                         player->current_list->current->singer);
    else if (player->current_list->play_status == MUSIC_PAUSED)
         MylerUI_SetTitle(player->ui, "已暂停播放“%s - %s”", player->current_list->current->name,
                         player->current_list->current->singer);

    // 歌词
    int lyrics_line_count = player->ui->main_window_frame.h - 1;
    char buf[MAX_STR_BUF];
    if (player->show_main_lyric)
        MylerUI_SetBottomLine(player->ui, AlignRight, LWHITE, "H: 帮助   ");
    if (player->current_list->play_status == MUSIC_PLAYING) {
        MylerLyrics_SetCurrentTime(player->current_list->current->lyrics, player->current_list->current_time);
        for (int i = -lyrics_line_count/2; i < lyrics_line_count/2; i++) {
            con_utf2con(buf, MylerLyrics_GetLyrics(player->current_list->current->lyrics, i));
            color_t color;
            if (i == 0) {
                color = UI_LYRICS_COLOR2;
                if (player->current_list->current->lyrics) {
                    if (player->show_main_lyric)
                        MylerUI_SetMainWindowLine(player->ui, lyrics_line_count / 2 + i, color, AlignCenter, ">>>>  %s  <<<<", buf);
                    else
                        MylerUI_SetBottomLine(player->ui, AlignCenter, UI_LYRICS_COLOR2, buf);
                } else if (player->show_main_lyric)
                    MylerUI_SetMainWindowLine(player->ui, lyrics_line_count / 2 + i, LRED, AlignCenter, ">>>>  未找到歌词文件  <<<<");
            } else if (player->show_main_lyric) {
                color = UI_LYRICS_COLOR1;
                MylerUI_SetMainWindowLine(player->ui, lyrics_line_count / 2 + i, color, AlignCenter, buf);
            }
        }
    }

    Myler_ListDisplayUpdate(player, &player->list_display, player->ui->list_frame.h - 1);

    // 界面
    MylerUI_Update(player->ui);
}

void Myler_Quit(MylerPlayer *player)
{
    MylerUI_Delet(player->ui, 1);
    for (int i = 0; i < player->list_count; i++)
        MylerList_Delet(player->list[i]);

    exit(0);
}
