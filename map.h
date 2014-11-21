#ifndef _MAP_H
#define _MAP_H

#define PASSED 2        //已走过的路
#define PASSABLE 1      //可通行
#define IMPASSABLE 0    //不可通行
#define ENDROUTE 9999   //路径边界
/* 初始化参数 */
#define CROSSNUM 2      //交叉路口数
#define CROSS1_X 1      //路口一中心坐标
#define CROSS1_Y 1
#define CROSS2_X 4      //路口二中心坐标
#define CROSS2_Y 1
#define MAPX 10         //地图x轴区块数
#define MAPY 10         //地图y轴区块数
#define ROADID 901      //路口起始编号

/* 坐标 */
typedef struct MapCooStruct
{
    int x;  //x轴
    int y;
}MapCoo;

/* 交叉路口信息 */
typedef struct CrossStruct
{
    int roadid; //路口编号
    MapCoo coo;
}Cross;

extern int RoadMapInit(int mapblock[][MAPY], Cross *cross); //道路地图初始化函数

#endif // _MAP_H
