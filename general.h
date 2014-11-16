#ifndef _GENERAL_H
#define _GENERAL_H

#include "camera.h"
#include "object.h"

#define WRITECYCLE_COMMU 15 //统计通信代价的写入周期（秒）
#define WRITECYCLE_STRENGTH 3 //节点强度写入周期（秒）

extern void OutputControl( Node *node );  //输出控制函数

#endif // _GENERAL_H


