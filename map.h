#ifndef _MAP_H
#define _MAP_H

#include "system.h"
/* 地图标记 */
#define IMPASSABLE      0           //不可通行
#define PASSABLE        1           //可通行
#define PASSED          2           //已走过的路

/* 方位 */
#define NORTH           1       //北
#define SOUTH           2       //南
#define EAST            3       //东
#define WEST            4       //西

/* 坐标 */
typedef struct MapCooStruct
{
    int x;
    int y;
}MapCoo;

/* 地图 */
typedef struct MapFrameStruct
{
    int abletag;
    MapCoo coo;
}MapFrame;

/* 交叉路口信息 */
typedef struct CrossStruct
{
    int roadid;         //路口编号
    MapCoo coo;         //路口中心坐标
    int halfwidth;      //车道半宽
    int halflength;     //车道半长
}Cross;

extern int RoadMapInit(MapFrame *mapf, Cross *cross, SystemPara *sys); //道路地图初始化函数

#endif // _MAP_H
