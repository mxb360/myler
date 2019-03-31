#include "myler_cmdline.h"
#include "myler_utils.h"
#include "console.h"

#define IS_SHORT_OPT(str)  ((str) && (str)[0] == '-' && (str)[1] && !(str)[2])
#define IS_LONG_OPT(str)   ((str) && (str)[0] == '-' && (str)[1] == '-' && (str)[2])

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

int MylerCmdLine_Init(MylerCmdLine *cmdline, int argc, char **argv)
{
    if (cmdline == NULL)
        return -1;
    memset(cmdline, 0, sizeof(MylerCmdLine));
    cmdline->argc = argc;
    cmdline->argv = argv;
    return MylerCmdLine_Resolve(cmdline);
}

void MylerCmdLine_PrintHelp(void)
{
    con_set_fcolor(LYELLOW);
    fprintf(myler_stdout, "  MylerPlayer 多功能音乐播放器V2.0\n\n");
    con_reset_color();
    fprintf(myler_stdout, "用法：myler [选项...] [文件...]\n\n");
    fprintf(myler_stdout, "选项:\n");
    fprintf(myler_stdout, "  -h, --help              显示此帮助信息后退出。\n");

    con_set_fcolor(LGREEN);
    fprintf(myler_stdout, "\n  如： myler \n\n");
    con_reset_color();
}

#define IS_STR(str1, str2)   !strcmp((str1), cmdline->argv[i]) || !strcmp((str2), cmdline->argv[i]) 

int  MylerCmdLine_Resolve(MylerCmdLine *cmdline)
{
    cmdline->music_name_count = 0;

    for (int i = 1; i < cmdline->argc; i++) {
        if (IS_STR("-h", "--help"))
            cmdline->have_help = true;
        else if (IS_SHORT_OPT(cmdline->argv[i]) || IS_LONG_OPT(cmdline->argv[i])) {
            Myler_PrintError("无法识别的选项“%s”", cmdline->argv[i]);
            return 1;
        }
        else
            cmdline->music_name[cmdline->music_name_count++] = cmdline->argv[i];
    }
    return 0;
}
