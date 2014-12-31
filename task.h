#ifndef _TASK_H
#define _TASK_H

#define FAILTON     3       //分配但无法执行
#define FAILEDT     2       //分配失败
#define RUNNING     1       //分配成功
#define STANDING    0       //未分配

#define MAXVALUE    10000   //Value极大值
#include "map.h"
#include "camera.h"

/* 任务信息 */
typedef struct TaskStruct
{
    int id;             //任务ID
    int state;          //任务分配状态
    int trans_flag;     //任务转移标记
    int execv_node;     //任务宿主节点
    float res_cost;     //任务资源消耗
    float qos_para;     //任务QoS阈值
    MapCoo coo;         //任务位置
    int *relatedgroup;  //任务相关组
}Task;

extern int TaskInit( Task *task, SystemPara *sys );
extern void TaskAllocation( Task *task, Node *node, SystemPara *sys );     //任务分配总控

#endif // _TASK_H
