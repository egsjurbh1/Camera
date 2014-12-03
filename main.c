/** \brief Camera Networks Software Framework
 *
 * \author chinglee
 * \update 141203
 * \version v1.1.2
 * \notice
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <windows.h>
#include "camera.h"
#include "map.h"
#include "object.h"
#include "general.h"
#include "system.h"

int main()
{
    int ret;
    SystemPara SysPara;                         //系统参数
    volatile MapFrame *MapBlock;                //地图
    volatile Cross *CrossRoad;                  //路口
    volatile Object *TrackObject;               //目标
    volatile Node *CameraNode;                  //节点

    /* 系统参数初始化 */
    ret = SystemInit(&SysPara);
    if(ret) {
        printf("SystemInit Error!\n");
        return -9;
    }

    /* 道路和地图配置初始化 */
    MapBlock = (MapFrame *)malloc( 2 * SysPara.Map.x_max * SysPara.Map.y_max * sizeof(MapFrame));
    CrossRoad = (Cross *)malloc( SysPara.Map.crossnum * sizeof(Cross));
    ret = RoadMapInit(MapBlock, CrossRoad, &SysPara);
    if(ret) {
        printf("RoadMapInit Error!\n");
        return -1;
    }

    /* 摄像头节点初始化 */
    CameraNode = (Node *)malloc( 3 * SysPara.Node.nodenum * sizeof(Node));  //防止内存不够！
    ret = CameraInit( CameraNode, CrossRoad, &SysPara );
    if(ret) {
        printf("CameraInit Error!\n");
        return -2;
    }

    /* 目标初始化 */
    TrackObject = (Object *)malloc( 3 * SysPara.Object.objectnum * sizeof(Object));
    ret = ObjectInit( TrackObject, &SysPara );
    if(ret) {
        printf("ObjectInit Error!\n");
        return -3;
    }
    printf("Init Success!\n");

    /* 摄像头节点总控 */
    CameraControl( CameraNode, TrackObject, &SysPara );
    sleep(1);

    /* 目标运动总控 */
    ObjectMovement( MapBlock, CrossRoad, TrackObject, &SysPara );
    sleep(1);

    /* 输出总控 */
    OutputControl( CameraNode, TrackObject, &SysPara );
    sleep(1);

    /* main主线程等待 */
    while(1)
    {
        sleep(1);
    }

    return 0;
}
