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
    SystemPara SysPara;                //ϵͳ����
    MapFrame *MapBlock;                //��ͼ
    Cross *CrossRoad;                  //·��
    Object *TrackObject;               //Ŀ��
    Node *CameraNode;                  //�ڵ�
    Task *AllocateTask;                //����

    /* ϵͳ������ʼ�� */
    ret = SystemInit(&SysPara);
    if(ret) {
        printf("SystemInit Error!\n");
        return -9;
    }

    /* �����ڴ�ռ� */
    MapBlock = (MapFrame *)malloc( 2 * SysPara.Map.x_max * SysPara.Map.y_max * sizeof(MapFrame));
    CrossRoad = (Cross *)malloc( 2 * SysPara.Map.crossnum * sizeof(Cross));
    CameraNode = (Node *)malloc( 5 * SysPara.Node.nodenum * sizeof(Node));  //��ֹ�ڴ治����
    TrackObject = (Object *)malloc( 3 * SysPara.Object.objectnum * sizeof(Object));
    AllocateTask = (Task *)malloc(3 * SysPara.Task.tasknum * sizeof(Task) );

    /* ��·�͵�ͼ���ó�ʼ�� */
    ret = RoadMapInit(MapBlock, CrossRoad, &SysPara);
    if(ret) {
        printf("RoadMapInit Error!\n");
        return -1;
    }

    /* ����ͷ�ڵ��ʼ�� */
    ret = CameraInit( CameraNode, CrossRoad, &SysPara );
    if(ret) {
        printf("CameraInit Error!\n");
        return -2;
    }

    /* Ŀ���ʼ�� */
    ret = ObjectInit( TrackObject, &SysPara );
    if(ret) {
        printf("ObjectInit Error!\n");
        return -3;
    }

    /* �����ʼ�� */
    ret = TaskInit( AllocateTask, &SysPara );
    if(ret) {
        printf("TaskInit Error!\n");
        return -4;
    }

    printf("Init Success! System Mode: %d\n", SysPara.system_mode);

    /* ����ͷ�ڵ��ܿ� */
    CameraControl( CameraNode, TrackObject, &SysPara );
    sleep(1);

    /* Ŀ���˶��ܿ� */
    ObjectMovement( MapBlock, CrossRoad, TrackObject, &SysPara );
    sleep(1);

    /* ��������ܿ� */
    TaskAllocation( AllocateTask, CameraNode, &SysPara );
    sleep(1);

    /* ����ܿ� */
    OutputControl( CameraNode, TrackObject, AllocateTask, &SysPara );
    sleep(1);

    /* ���߳�ģʽ�ж� */
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
        system("taskallocation.txt");   //���ļ����
        break;
    default:
        printf("End: No Running Mode!\n");
        break;
    }

    return 0;
}
