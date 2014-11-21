/** \brief 摄像头网络目标跟踪仿真
 *
 * \author lq
 * \update 141121
 * \version v1.0.0
 * \notice
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "camera.h"
#include "map.h"
#include "object.h"
#include "general.h"

int main()
{
    int ret;

    /*MapBlock[i][j]存储地图坐标（i,j）*/
    volatile int MapBlock[MAPX][MAPY];    //类型 IMPASSABLE：不可通行；PASSABLE：可通行
    /*crossroad[i]存储交叉路口i信息*/
    volatile Cross CrossRoad[CROSSNUM];
    volatile Object TrackObject[OBJECTNUM];   //跟踪目标
    volatile Node CameraNode[NODENUM];  //摄像头节点
    /*NodeMsg[i][j]存储节点j发给节点i的消息*/
    volatile int NodeMsg[NODENUM][NODENUM];   //消息类型 NONEACTION：无；SUCCESSA：移交成功；FAILA：非相邻节点；
    /*TaskMsg[i][j]存储节点i收到目标j的任务消息*/
    volatile TaskInfo TaskMsg[NODENUM][OBJECTNUM];

    /* 道路和地图配置初始化 */
    ret = RoadMapInit(MapBlock, CrossRoad);
    if(ret != 0) {
        printf("RoadMapInit Error!\n");
        return -2;
    }

    /* 摄像头节点初始化 */
    ret = CameraInit( CameraNode, NodeMsg, TaskMsg );
    if(ret != 0) {
        printf("CameraInit Error!\n");
        return -3;
    }

    /* 目标初始化 */
    ret = ObjectInit( TrackObject );
    if(ret != 0) {
        printf("ObjectInit Error!\n");
        return -4;
    }
    printf("Init Success!\n");

    /* 摄像头节点总控 */
    CameraControl( CameraNode, NodeMsg, TaskMsg, TrackObject );
    sleep(1);

    /* 目标运动总控 */
    ObjectMovement( MapBlock, TrackObject, EXECUTEACTION );
    sleep(1);

    /* 输出结果总控 */
    OutputControl( CameraNode );
    sleep(1);

    /* main主线程等待 */
    while(1)
    {
        sleep(1);
    }

    return 0;
}
