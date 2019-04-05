#include "myler_cmdline.h"
#include "myler_utils.h"
#include "console.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define IS_SHORT_OPT(str)  ((str) && (str)[0] == '-' && (str)[1] && !(str)[2])
#define IS_LONG_OPT(str)   ((str) && (str)[0] == '-' && (str)[1] == '-' && (str)[2])

int MylerCmdLine_Init(MylerCmdLine *cmdline, int argc, char **argv)
{
    if (cmdline == NULL)
        return -1;
    memset(cmdline, 0, sizeof(MylerCmdLine));
    cmdline->argc = argc;
    cmdline->argv = argv;

    cmdline->volume = -1;

    return MylerCmdLine_Resolve(cmdline);
}

void MylerCmdLine_PrintHelp(void)
{
    con_set_fcolor(LYELLOW);
    fprintf(myler_stdout, "Myler Player 多功能音乐播放器 V2.0\n\n");
    con_reset_color();
    fprintf(myler_stdout,
        "用法：myler [选项...] [文件...]\n\n"
        "文件:\n"
        "   指的是音乐(MP3)文件，支持多个文件、通配符、相对路径、绝对路径；\n"
        "   添加的文件将会被添加到临时列表中，文件名可重复，自动忽略非MP3文件。\n\n"
        "选项:\n"
        "  -h, --help               显示此帮助信息后退出。\n"
        "  -V, --version            显示版本信息后退出。\n"
        "  -e, --exit               列表播放完成后退出。\n"
        "  -m, --mini               迷你版界面（仅含进度状态窗口）。\n"
        "  -v, --volume VOL         设置播放音量为VOL，VOL取值(0-100)。\n"
        "  --stop                   进入播放器后暂停音乐的播放。\n\n"
        "  -O, --order              设置播放模式为：顺序播放。\n"
        "  -L, --loop               设置播放模式为：列表循环。\n"
        "  -R, --repeat             设置播放模式为：单曲循环。\n"
        "  -S, --shuffle            设置播放模式为：随机播放。\n\n" 
        "  * -s, --search           进入界面后立即搜索。\n"
        "  * --search-by-netease    设置搜索引擎为：网易云音乐。\n"
        "  * --search-by-qq         设置搜索引擎为：QQ音乐。\n"
        "  * --search-by-kugou      设置搜索引擎为：酷狗音乐。\n"
        "  * --search-by-local      设置搜索引擎为：本地音乐。\n"
        "  * --search-name          设置搜索类型：歌名/歌手。\n"
        "  * --search-list          设置搜索类型：歌单。\n"
        "  * --search-album         设置搜索类型：专辑。\n"
        "  * --search-lyric         设置搜索类型：歌词。\n"
        "  * --search-words [words] 设置搜索关键字为word，默认空字符串。\n\n"
        "  * -n, --no-ui            不绘制界面，也不再响应按键，若要响应按键，请追加“--use-key-when-no-ui”。\n"
        "  --no-color               禁用界面、输出提示颜色。\n"
        "  * --use-key-when-no-ui   在不显示界面的情况下任响应按键。\n");

    fprintf(myler_stdout, " \n注意：\n  如果选项有冲突，程序以后给出的选项为准。\n  带*的选项暂未实现。\n");
    fprintf(myler_stdout, " \n示例：\n");
    con_set_fcolor(LGREEN);
    fprintf(myler_stdout, "  myler -v 20 --mini --exit hello.mp3\n");
    con_reset_color();
    fprintf(myler_stdout, "  以迷你界面播放hello.mp3，设置音量为20%%并在播放完成之后退出。\n");
}

#define IS_STR(str1, str2)        !strcmp((str1), cmdline->argv[i]) || !strcmp((str2), cmdline->argv[i]) 
#define GET_INT_ARG(value, a, b)  (i + 1 < cmdline->argc && sscanf(cmdline->argv[i + 1], "%d", &(value)) == 1 && cmdline->volume >= (a) && cmdline->volume <= (b))     

int  MylerCmdLine_Resolve(MylerCmdLine *cmdline)
{
    cmdline->music_name_count = 0;

    const char *unknown_opt = NULL;
    const char *invalid_arg = NULL;
    for (int i = 1; i < cmdline->argc; i++) {
        if (IS_STR("-h", "--help"))
            cmdline->have_help = true;
        else if (IS_STR("-V", "--version"))
            cmdline->have_version = true;
        else if (IS_STR("-e", "--exit"))
            cmdline->have_exit = true;
        else if (IS_STR("-m", "--mini"))
            cmdline->have_mini = true;
        else if (IS_STR("-v", "--volume")) {
            if (!GET_INT_ARG(cmdline->volume, 0, 100) && !invalid_arg) 
                invalid_arg = cmdline->argv[i];
        } else if (IS_STR("", "--stop"))
            cmdline->have_stop = true;
        else if (IS_STR("-O", "--order"))
            cmdline->have_order = true, cmdline->have_loop = cmdline->have_repeat = cmdline->have_shuffle = false;
        else if (IS_STR("-L", "--loop"))
            cmdline->have_loop = true, cmdline->have_order = cmdline->have_repeat = cmdline->have_shuffle = false;
        else if (IS_STR("-R", "--repeat"))
            cmdline->have_repeat = true, cmdline->have_loop = cmdline->have_order = cmdline->have_shuffle = false;
        else if (IS_STR("-S", "--shuffle"))
            cmdline->have_shuffle = true, cmdline->have_loop = cmdline->have_repeat = cmdline->have_order = false;
        
        else if (IS_STR("", "--no-color"))
            con_use_color(false);
        
        else if ((IS_SHORT_OPT(cmdline->argv[i]) || IS_LONG_OPT(cmdline->argv[i])) && !unknown_opt) 
            unknown_opt = cmdline->argv[i];
        else
            cmdline->music_name[cmdline->music_name_count++] = cmdline->argv[i];
    }
    if (unknown_opt) {
        Myler_PrintError("无法识别的选项“%s”", unknown_opt);
        return 1;
    } else if (invalid_arg) {
        Myler_PrintError("选项“%s”参数非法", invalid_arg);
        return 1;
    }
    return 0;
}
