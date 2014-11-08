/** \brief 摄像头网络目标跟踪仿真
 *
 * \author lq
 * \update 141105
 * \version v0.4.0
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "camera.h"
#include "map.h"
#include "object.h"

int main()
{
    FILE *fp;
    int i, j, ret;
    /*MapBlock[i][j]存储地图坐标（i,j）*/
    volatile int MapBlock[MAPX][MAPY];    //类型 IMPASSABLE：不可通行；PASSABLE：可通行
    /*crossroad[i]存储交叉路口i信息*/
    Cross CrossRoad[CROSSNUM];
    Object TrackObject[OBJECTNUM];   //跟踪目标
    Node CameraNode[NODENUM];  //摄像头节点
    /*NodeMsg[i][j]存储节点j发给节点i的消息*/
    volatile int NodeMsg[NODENUM][NODENUM];   //消息类型 NONEACTION：无；SUCCESSA：移交成功；FAILA：非相邻节点；
    /*TaskMsg[i][j]存储节点i收到目标j的任务消息*/
    volatile TaskInfo TaskMsg[NODENUM][OBJECTNUM];

    /* 道路和地图配置初始化 */
    ret = RoadMapInit(MapBlock, CrossRoad);
    if(ret != 0) {
        printf("RoadMapInit Error!\n");
        return 0;
    }

    /* 摄像头节点初始化 */
    ret = CameraInit( CameraNode, NodeMsg, TaskMsg );
    if(ret != 0) {
        printf("CameraInit Error!\n");
        return 0;
    }

    /* 目标初始化 */
    ret = ObjectInit( TrackObject );
    if(ret != 0) {
        printf("ObjectInit Error!\n");
        return 0;
    }
    printf("Init Success!\n");

    /* 摄像头节点总控 */
    CameraControl( CameraNode, NodeMsg, TaskMsg, TrackObject );
    sleep(1);
    /* 目标运动总控 */
    ObjectMovement( MapBlock, TrackObject, EXECUTEACTION );
    sleep(1);

    while(1)
    {
        /**< 输出节点关系强度矩阵到文件*/
        fp = fopen("nodestrength.txt", "w+");
        for(i = 0; i < NODENUM; i++)
        {
            for(j = 0; j < NODENUM; j++)
                fprintf(fp, "%.1f\t", CameraNode[i].rstrength[j]);
            fprintf(fp, "\n");
        }
        fprintf(fp, "\n");
        fclose(fp);
        sleep(3);
    }

    return 0;
}






