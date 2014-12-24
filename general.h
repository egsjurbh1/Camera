#ifndef _GENERAL_H
#define _GENERAL_H

#include "camera.h"
#include "object.h"
#include "task.h"
#include "system.h"

#define ONUM                        4       //����ļ�����
#define FILEBUFFER                  20      //������

/* �ļ�����̲߳����ṹ�� */
typedef struct FileThreadArgStruct
{
    Node *node;
    Object *object;
    SystemPara *sys;
}FileThreadArg;

extern void OutputControl( Node *node, Object *obj, Task *task, SystemPara *sys );    //������ƺ���

#endif // _GENERAL_H


