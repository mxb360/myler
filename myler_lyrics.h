#ifndef __MYLER_LYRICS_H__
#define __MYLER_LYRICS_H__

#define MAX_LYRIC_LINE   1000
#define MAX_LYRIC_TIME   0xffffffff

typedef struct _MylerLyricsWords {
    unsigned long time;
    const char *words;
} MylerLyricsWords;

typedef struct _MylerLyrics {
    const char *file_name;
    MylerLyricsWords *lyrics;
    int  lyrics_cnt;
    int currect_lyrics;
    int  offset;
    char **words;
    int  words_cnt;
} MylerLyrics;

MylerLyrics *MylerLyrics_GetLyricsByFile(const char *file);
void  MylerLyrics_Print(MylerLyrics *lyrics);
void  MylerLyrics_Free(MylerLyrics *lyrics);
int   MylerLyrics_SetCurrentTime(MylerLyrics *lyrics, unsigned long current_time);
const char *MylerLyrics_GetLyrics(MylerLyrics *lyrics, int line);

#endif  /* !__MYLER_LYRICS_H__ */
