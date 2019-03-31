#include <stdio.h>
#include <signal.h>

#include "myler.h"

bool break_loop = false;

void break_by_ctrl_c(int sig)
{
    break_loop = true;
    signal(SIGINT, break_by_ctrl_c);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, break_by_ctrl_c);

    MylerPlayer player;
    int err_code;

    if ((err_code = Myler_Init(&player, argc, argv))) {
        if (err_code < 0)
            Myler_PrintError("播放器初始化失败!");
        return err_code;
    }

    while (!break_loop)
    {
        Myler_Update(&player);
        con_sleep(100);
    }

    Myler_Quit(&player);
    return 0;
}
