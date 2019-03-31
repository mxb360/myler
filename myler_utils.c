#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <io.h>

#include <direct.h>
#include <stdlib.h>

#include "myler_utils.h"
#include "console.h"

void Myler_Abort(int line, const char *func, const char *file, const char *op)
{
    fprintf(stderr, "[%s: %d] Error: In function \"%s\": \"%s\" Assert Faild.\n", file, line, func, op);
    *(int *)0 = 0;
    exit(-1);
}

void Myler_PrintError(const char *format, ...)
{
    char buf[MAX_STR_BUF];
    va_list ap;
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);

    con_set_fcolor(LRED);
    printf("myler: 错误：%s。\n", buf);
    con_reset_color();
    printf("myler: 输入“--help”可查看帮助。\n");
}

/* 获得指定目录下的所有文件名 */
int GetFiles(char *files[], int n, const char *dir)
{
    struct _finddata_t file;
    char buf[256];
    long handle;
    int i, total = 0;
    char *path = NULL, *path_bak = _getcwd(NULL, 0);

    strcpy(buf, dir);
    for (i = strlen(buf); i >= 0 && buf[i] != '\\' && buf[i] != '/'; i--);
    if (i >= 0)
    {
        buf[i] = 0;
        if (_chdir(buf) < 0) {
            free(path_bak);
            return -1;
        }
    }  
    path = _getcwd(NULL, 0);
    _chdir(path_bak);
    free(path_bak);

    handle = _findfirst(dir, &file);
    if (handle == -1) {
        free(path);
        return -1;
    }
    int a = 0;
    do {
        files[total] = (char *)malloc(strlen(file.name) + strlen(path) + 10);
        if (files[total] != NULL && file.name[0] != '.') {
            strcpy(files[total], path);
            strcat(files[total], "\\");
            strcat(files[total++], file.name);
        }
    } while (total < n && !(a = _findnext(handle, &file)));
    
    free(path);
    return total;
}
