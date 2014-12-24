#ifndef _GENERAL_H
#define _GENERAL_H

#include "camera.h"
#include "object.h"
#include "task.h"
#include "system.h"

#define ONUM                        4       //输出文件数量
#define FILEBUFFER                  20      //缓冲数

/* 文件输出线程参数结构体 */
typedef struct FileThreadArgStruct
{
    Node *node;
    Object *object;
    SystemPara *sys;
}FileThreadArg;

extern void OutputControl( Node *node, Object *obj, Task *task, SystemPara *sys );    //输出控制函数

#endif // _GENERAL_H


