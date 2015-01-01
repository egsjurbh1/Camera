/** \brief Camera Networks Software Framework
 *
 * \author chinglee
 * \update 150101
 * \version v1.3.8
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
#include "task.h"

int main()
{
    int ret;
    SystemPara SysPara;                //系统参数
    MapFrame *MapBlock;                //地图
    Cross *CrossRoad;                  //路口
    Object *TrackObject;               //目标
    Node *CameraNode;                  //节点
    Task *AllocateTask;                //任务

    /* 系统参数初始化 */
    ret = SystemInit(&SysPara);
    if(ret) {
        printf("SystemInit Error!\n");
        return -9;
    }

    /* 分配内存空间 */
    MapBlock = (MapFrame *)malloc( 2 * SysPara.Map.x_max * SysPara.Map.y_max * sizeof(MapFrame));
    CrossRoad = (Cross *)malloc( 2 * SysPara.Map.crossnum * sizeof(Cross));
    CameraNode = (Node *)malloc( 5 * SysPara.Node.nodenum * sizeof(Node));  //防止内存不够！
    TrackObject = (Object *)malloc( 3 * SysPara.Object.objectnum * sizeof(Object));
    AllocateTask = (Task *)malloc(3 * SysPara.Task.tasknum * sizeof(Task) );

    /* 道路和地图配置初始化 */
    ret = RoadMapInit(MapBlock, CrossRoad, &SysPara);
    if(ret) {
        printf("RoadMapInit Error!\n");
        return -1;
    }

    /* 摄像头节点初始化 */
    ret = CameraInit( CameraNode, CrossRoad, &SysPara );
    if(ret) {
        printf("CameraInit Error!\n");
        return -2;
    }

    /* 目标初始化 */
    ret = ObjectInit( TrackObject, &SysPara );
    if(ret) {
        printf("ObjectInit Error!\n");
        return -3;
    }

    /* 任务初始化 */
    ret = TaskInit( AllocateTask, &SysPara );
    if(ret) {
        printf("TaskInit Error!\n");
        return -4;
    }

    printf("Init Success! System Mode: %d\n", SysPara.system_mode);

    /* 摄像头节点总控 */
    CameraControl( CameraNode, TrackObject, &SysPara );
    sleep(1);

    /* 目标运动总控 */
    ObjectMovement( MapBlock, CrossRoad, TrackObject, &SysPara );
    sleep(1);

    /* 任务分配总控 */
    TaskAllocation( AllocateTask, CameraNode, &SysPara );
    sleep(1);

    /* 输出总控 */
    OutputControl( CameraNode, TrackObject, AllocateTask, &SysPara );
    sleep(1);

    /* 主线程模式判断 */
    switch(SysPara.system_mode)
    {
    case TRACK_MODE:
        while(1)
        {
            sleep(1);
        }
        break;
    case TASK_MODE:
        printf("End: Task Allocation Success!\n");
        system("taskallocation.txt");   //打开文件结果
        break;
    default:
        printf("End: No Running Mode!\n");
        break;
    }

    return 0;
}
