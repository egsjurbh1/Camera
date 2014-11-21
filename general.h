#ifndef _GENERAL_H
#define _GENERAL_H

#include "camera.h"
#include "object.h"
#include "map.h"

#define WRITECYCLE_COMMU    15      //统计通信代价的写入周期（秒）
#define WRITECYCLE_STRENGTH 3       //节点强度写入周期（秒）

#define BUFSIZE             20      //缓存大小
#define CONFIGNUM           7       //配置参数数量
#define MAXCROSSNUM         10      //最大路口数量
#define MAXOBJECTNUM        20      //最大目标数量
#define MAXCAMERANUM        20      //最大节点数量

//配置文件解析结构体
typedef struct FileBufStruct{
	char keyname[BUFSIZE];          //键名
	char keyvalue[BUFSIZE];         //键值
}FileBuf;

//节点配置参数结构体
typedef struct CameraConfigStruct{
    int nodenum;                    //节点数
    float strengthinit;             //strength初值
    float strengthincre;            //strength增量
    float strengthratio;            //系数
    float pvalue;                   //公式参数概率ω
    float tvalue;                   //公式参数阈值
    MapCoo cameracoo[MAXCAMERANUM]; //节点初始化位置
}CameraCS;

//目标配置参数结构体
typedef struct ObjectConfigStruct{
    int objectnum;                  //目标数
    MapCoo objectcoo[MAXOBJECTNUM]; //目标初始化位置
}ObjectCS;

//地图配置参数结构体
typedef struct MapConfigStruct{
    int mapx;                       //地图x轴区块数
    int mapy;                       //地图y轴区块数
    int crossnum;                   //交叉路口数
    MapCoo crosscoo[MAXCROSSNUM];   //路口中心坐标
}MapCS;

//全局配置参数结构体
typedef struct ConfigParameterStruct{
    CameraCS Camera;
    ObjectCS Object;
    MapCS Map;
}CfgStruct;

extern void OutputControl( Node *node );        //输出控制函数
extern int SystemInit(CfgStruct *cfg);          //系统初始化
#endif // _GENERAL_H


