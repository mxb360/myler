#include <stdio.h>
#include <signal.h>

#include "myler.h"

static bool break_loop = false;


// Ctrl-C 退出
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
    
    // 初始化播放器
    if ((err_code = Myler_Init(&player, argc, argv))) {
        if (err_code < 0)
            Myler_PrintError("播放器初始化失败!");
        return err_code;
    }

    // 循环更新播放器状态
    while (!break_loop) {
        Myler_Update(&player);
        con_sleep(100);
    }

    // 退出播放器
    Myler_Quit(&player, false);
    return 0;
}
