#include <string.h>
#include <stdlib.h>

#include "myler_list.h"
#include "myler_utils.h"


static void GetNameAndSinger(const char *file_name, char *name, char *singer, int mode)
{
    int name_pos = 0, singer_pos = 0;

    const char *point = strrchr(file_name, '.');
    if (mode) {
        char *p = singer;
        singer = name;
        name = p;
    }

    while (*file_name == ' ')
        file_name++;
    while (*file_name && *file_name != '-' && file_name != point)
        name[name_pos++] = *file_name++;
    while (name_pos > 0 && name[name_pos - 1] == ' ')
        name_pos--;

    if (*file_name == '-')
        file_name++;

    while (*file_name == ' ')
        file_name++;
    while (file_name != point)
        singer[singer_pos++] = *file_name++;
    while (singer_pos && singer[singer_pos - 1] == ' ')
        singer_pos--;

    if (name_pos)
        name[name_pos] = 0;
    else
        strcpy(name, "<未知歌名>");
    if (singer_pos)
        singer[singer_pos] = 0;
    else
        strcpy(singer, "<未知歌手>");
}

static const char *GetFileName(const char *path)
{
    int i = strlen(path);

    while (--i >= 0 && path[i] != '\\' && path[i] != '/');
    return i ? path + i + 1 : path;
}

MylerList *MylerList_CreateList(const char *list_name)
{
    MylerList *list = (MylerList *)malloc(sizeof(MylerList));
    if (list) {
        strcpy(list->list_name, list_name);
        list->head = list->tail = NULL;
        list->music_count = 0;
        list->current = NULL;
        list->is_open = false;
        list->play_status = MUSIC_NOT_READY;
        list->is_play_end = true;
        list->replay = false;
        list->current_time = 0;
        list->is_expand = false;
    }
    return list;
}

void MylerList_Delet(MylerList *list)
{
    Myler_Assert(list);

    MylerListNode *node, *node2;
    node = list->head;
    for (int i = 0; i < list->music_count; i++) {
        music_delete(node->music);
        MylerLyrics_Free(node->lyrics);
        node2 = node;
        node = node->next;
        free(node2);
    }
}

MylerListNode *MylerList_AddLocalMusic(MylerList *list, const char *file_path)
{
    MylerListNode *node;

    if (!(node = (MylerListNode *)malloc(sizeof(MylerListNode)))) 
        return NULL;

    node->list = list;
    node->next = NULL;
    node->prev = list->tail;
    node->music = NULL;
    node->length = -1;
    node->lyrics = NULL;
    GetNameAndSinger(GetFileName(file_path), node->name, node->singer, 0);

    if (!list->head)
        list->head = node;
    if (list->tail)
        list->tail->next = node;
    list->tail = node;

//    music_delete(music);
    list->music_count++;
    strcpy(node->path, file_path);

    return node;
}

void MylerList_ClearCurrent(MylerList *list)
{
    Myler_Assert(list);
    if (list->current && list->current->music) {
        music_delete(list->current->music);
        MylerLyrics_Free(list->current->lyrics);
        list->current->music = NULL;
        list->is_open = false;
        list->is_play_end = false;
    }
}

MylerListNode *MylerList_SetCurrent(MylerList *list, int index)
{
    Myler_Assert(list);
    Myler_Assert(index >= 0 && index < list->music_count);

    MylerListNode *node = list->head;
    MylerList_ClearCurrent(list);

    for (int i = 0; i < index; i++)
        node = node->next;

    list->current = node;
    list->is_play_end = false;
    list->replay = true;

    return node;
}

/* 生成音乐文件名对应的歌词文件名 */
static char *get_lyric_file_name(char *file_name)
{
    int len = strlen(file_name);
    while (--len > 0 && file_name[len] != '.');
    if (len == 0)
        return "";
    strcpy(file_name + len, ".lrc");
    return file_name;
}

int MylerList_Open(MylerList *list)
{
    Myler_Assert(list);

    if (!list->current)
        return -1;
    if (!(list->current->music = music_create(list->current->path))) {
        return -2;
    }

    char buf[MAX_STR_BUF];
    list->is_open = true;
    list->is_play_end = false;
    list->current->length = music_get_length(list->current->music);
    list->current->lyrics = MylerLyrics_GetLyricsByFile(get_lyric_file_name(strcpy(buf, list->current->path)));

    return 0;
}

int MylerList_Play(MylerList *list)
{
    Myler_Assert(list);

    if (!list->current || !list->is_open)
        return -1;
    list->is_play_end = false;

    if (music_play(list->current->music, 0, 0, 0))
        return -2;
    return 0;
}

int MylerList_Pause(MylerList *list)
{
    Myler_Assert(list);

    if (!list->current || !list->is_open)
        return -1;
    return music_pause(list->current->music);
}

int MylerList_Resume(MylerList *list)
{
    Myler_Assert(list);

    if (!list->current || !list->is_open)
        return -1;
    return music_resume(list->current->music);
}

int MylerList_SetNext(MylerList *list, int play_mode)
{
    Myler_Assert(list);

    if (!list->current)
        return -1;

    list->replay = true;
    MylerList_ClearCurrent(list);

    if (play_mode == PlayRepeatOne) 
        return 0;
    else if (list->current == list->tail && play_mode == PlayInOrder)
        list->is_play_end = true, list->replay = false;
    else if (list->current == list->tail && play_mode == PlayListLoop)
        list->current = list->head;
    else if (list->current != list->tail && play_mode != PlayShuffle) 
        list->current = list->current->next;
    else if (play_mode == PlayShuffle)
        MylerList_SetCurrent(list, rand() % list->music_count);
    return 0;
}

int MylerList_SetPrev(MylerList *list)
{
    Myler_Assert(list);

    if (!list->current)
        return -1;
    MylerList_ClearCurrent(list);
    if (list->is_play_end)
        list->current = list->tail;
    else if (list->current != list->head)
        list->current = list->current->prev;
    list->replay = true;
    return 0;
}

void MylerListDisplay_SetList(MylerListDisplay *list_display, MylerList *lists[], int list_count)
{
    Myler_Assert(list_display);
    Myler_Assert(lists);

    MylerListDisplayNode *last_node = NULL;

    for (int line = 0; line < list_count; line++) {
        MylerListDisplayNode *node = (MylerListDisplayNode *)malloc(sizeof(struct MylerListDisplayNode));
        if (!node)
            return;
        
        node->list = lists[line];
        node->type = IsListType;
        node->prev = last_node;
        node->music = NULL;
        node->str1 = lists[line]->list_name;
        node->str2 = NULL;
        if (last_node)
            last_node->next = node;
        else
            list_display->head = node;

        last_node = node;
        list_display->display_list_count++;
        if (lists[line]->is_expand) {
            MylerListNode *music = lists[line]->head;
            for (int count = 0; count < lists[line]->music_count; count++) {
                MylerListDisplayNode *node = (MylerListDisplayNode *)malloc(sizeof(struct MylerListDisplayNode));
                if (!node)
                    return;
                
                node->list = lists[line];
                node->type = IsListNodeType;
                node->prev = last_node;
                node->music = music;
                node->str1 = music->name;
                node->str2 = music->singer;
                last_node->next = node;

                last_node = node;
                music = music->next;
                list_display->display_list_count++;
            }
        }
    }
    list_display->display_list = list_display->head;
    last_node->next = NULL;
    list_display->tail = last_node;
    list_display->current_choice = list_display->head;
    list_display->default_choice = NULL;
}

void MylerListDispaly_Delete(MylerListDisplay *list_display)
{
    if (list_display == NULL)
        return;
    MylerListDisplayNode *node = list_display->head;
    while (node)
    {
        MylerListDisplayNode *node2 = node;
        node = node->next;
        free(node2);
    }
}
