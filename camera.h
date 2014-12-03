#ifndef _CAMERA_H
#define _CAMERA_H

#define MAXNODE         20      //node max
#define MAXOBJECT       40      //object max

/* 通信 */
#define TASKCOMMU       1       //任务发布通信
#define FEEDBACKCOMMU   2       //反馈消息通信
#define OUTPUTIC        3       //输出瞬时通信量

/* 消息、任务类型 */
#define NONEA           0       //初始化
#define SUCCESSA        1       //移交成功
#define FAILA           2       //非相邻节点

#define TRACKTASK       1       //跟踪任务


#include "map.h"
#include "object.h"
#include "system.h"

/* 任务消息 */
typedef struct TaskInfoStruct
{
    int nodeid;         //消息源节点
    int objectid;       //目标
    int tasktype;       //消息类型 NONEA：无任务；TRACKTASK：跟踪任务；
}TaskInfo;

/* 任务集合 */
typedef struct TaskSetStruct
{
    int objectid;                   //目标编号
    int postnodeid;                 //消息源节点编号
}TaskSet;

/* 节点信息 */
typedef struct NodeStruct
{
    int nodeid;                     //节点编号
    MapCoo coo;                     //节点位置坐标
    int fovlength;                  //视域长度
    int fovhalfwidth;               //视域半宽
    int direction;                  //监控方向
    int commu_mode;                 //发布任务的方式 MULTICAST,BROADCAST
    float load;                     //节点负载
    float utility;                  //节点效用
    float *rstrength;               //节点关系强度表
    TaskSet *SdetectO;              //目标跟踪集
    TaskSet *StrackO;               //跟踪任务集
}Node;

/* 节点反馈消息处理线程参数结构体 */
typedef struct CameraThreadArgStruct
{
    Node *node;
    Object *object;
    SystemPara *sys;
}CameraThreadArg;

extern int CameraInit(Node *node, Cross *cross, SystemPara *sys);                                       //节点初始化函数
extern void CameraControl( Node *node, Object *obj, SystemPara *sys );                                  //节点总控函数
extern float count_communication(int flag, int type, float *ic, float taskcost, float feedbackcost);    //通信代价统计

#endif // _CAMERA_H
