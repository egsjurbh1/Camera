#ifndef _CAMERA_H
#define _CAMERA_H

#define NODENUM 5           //节点数
/* 关系强度 */
#define STRENGTHINIT 1.3    //strength初值
#define STRENGTHINCRE 1.0   //增量
#define STRENGTHRATIO 0.1   //系数
#define PVALUE 0.01         //公式参数概率ω
#define TVALUE 1.0          //公式参数阈值

/* 消息、任务类型 */
#define NONEA 0             //初始化
#define SUCCESSA 1          //移交成功
#define FAILA 2             //非相邻节点
#define TRACKTASK 11        //跟踪任务
#define MULTICAST 1         //任务发布模式：组播
#define BROADCAST 2         //任务发布模式：广播

/* 编号 */
#define NONEID 0            //无编号
#define NODEID 101          //节点起始编号

#include "map.h"
#include "object.h"

/* 任务消息 */
typedef struct TaskInfoStruct
{
    int nodeid;     //消息源节点
    int objectid;   //目标
    int tasktype;   //消息类型 NONEA：无任务；TRACKTASK：跟踪任务；
}TaskInfo;

/* 节点信息 */
typedef struct NodeStruct
{
    int nodeid;                     //节点编号
    float rstrength[NODENUM];       //节点关系强度表
    int SdetectO[OBJECTNUM][2];     //目标跟踪集,位置0存目标编号，位置1存源节点号
    int StrackO[OBJECTNUM][2];      //跟踪任务集
    MapCoo coo;
}Node;

/* 节点反馈消息处理线程参数结构体 */
typedef struct CameraThreadArgStruct
{
    int **nodemsg;
    Node *node;
    Object *object;
    TaskInfo **taskmsg;
}CameraThreadArg;

extern int CameraInit( Node *node, int nodemsg[][NODENUM], TaskInfo taskmsg[][OBJECTNUM]);                      //节点初始化函数
extern void CameraControl( Node *node, int nodemsg[][NODENUM], TaskInfo taskmsg[][OBJECTNUM], Object *obj );    //节点总控函数
extern int count_communication(int flag);                                                                       //通信代价统计

#endif // _CAMERA_H
