#ifndef __CMDLINE_H__
#define __CMDLINE_H__

#include "myler_config.h"

typedef struct MylerCmdLine {
    int argc;
    char **argv;

    bool have_help;
    const char *music_name[100];
    int music_name_count;
} MylerCmdLine;

int MylerCmdLine_Init(MylerCmdLine *cmdline, int argc, char **argv);
int MylerCmdLine_Resolve(MylerCmdLine *cmdline);
void MylerCmdLine_PrintHelp(void);

#endif  /* !__CMDLINE_H__ */
