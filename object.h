#ifndef _OBJECT_H
#define _OBJECT_H

/* 运动模式 */
#define PASSIVEMODE     1      //路径引导
#define ACTIVEMODE      2      //随机寻路
#define MAXROUTE        50     //最大路径存储数
#define PERCENT         4      //1:4概率,find_next函数随机决策

#include "map.h"
#include "system.h"

/* 目标信息 */
typedef struct ObjectStruct
{
    int objectid;               //目标编号
    int speed;                  //运动速度
    int direction;              //运动方位（在mode为ACTIVEEMODE时有效）
    int mode;                   //运动模式
    float distance;             //和节点的距离
    MapCoo coo;                 //当前坐标
    int route[MAXROUTE][2];     //路径信息（在mode为PASSIVEEMODE时有效）
    int r_size;                 //路径size
}Object;

/* 目标运动线程参数结构体 */
typedef struct MoveArgStruct
{
    MapFrame *mapf;
    Object *obj;
    Cross *cross;
    SystemPara *sys;
}MoveArg;

extern int ObjectInit( Object *obj, SystemPara *sys ); //目标初始化函数
extern void ObjectMovement( MapFrame *mapf, Cross *cross, Object *obj, SystemPara *sys );   //目标运动控制函数

#endif // _OBJECT_H
