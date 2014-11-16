#ifndef _OBJECT_H
#define _OBJECT_H

#define OBJECTNUM 2 //目标数
#define OBJECTID 601 //目标起始编号
#define NONEOID 0 //无编号

/* 运动控制 */
#define STOPACTION   0  //停止
#define EXECUTEACTION 1     //执行
/* 运动模式 */
#define PASSIVEMODE  11   //路径引导
#define ACTIVEMODE   12   //随机寻路
#define PERCENT 4         ////1:4概率,find_next函数随机决策

#include "map.h"

/* 目标信息 */
typedef struct ObjectStruct
{
    int objectid;   //目标编号
    float speed;    //运动速度
    int mode;       //运动模式
    MapCoo coo;     //当前坐标
    MapCoo destcoo; //目的地坐标
}Object;

/* 目标运动线程参数结构体 */
typedef struct MoveArgStruct
{
    int **mapblock;
    Object *obj;
}MoveArg;

extern int ObjectInit( Object *obj ); //目标初始化函数
extern void ObjectMovement( int mapb[][MAPY], Object *obj, int configarg );   //目标运动控制函数

#endif // _OBJECT_H
