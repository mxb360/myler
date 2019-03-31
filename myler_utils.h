#ifndef __MYLER_UTILS_H__
#define __MYLER_UTILS_H__

#include "myler_config.h"

#define Myler_Assert(op)  (op ? (void)0 : Myler_Abort(__LINE__, __func__, __FILE__, #op))

void Myler_Abort(int line, const char *func, const char *file, const char *op);
int GetFiles(char *files[], int n, const char *dir);
void Myler_PrintError(const char *format, ...);
#endif
