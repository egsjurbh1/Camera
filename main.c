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
    SystemPara SysPara;                         //ϵͳ����
    volatile MapFrame *MapBlock;                //��ͼ
    volatile Cross *CrossRoad;                  //·��
    volatile Object *TrackObject;               //Ŀ��
    volatile Node *CameraNode;                  //�ڵ�

    /* ϵͳ������ʼ�� */
    ret = SystemInit(&SysPara);
    if(ret) {
        printf("SystemInit Error!\n");
        return -9;
    }

    /* ��·�͵�ͼ���ó�ʼ�� */
    MapBlock = (MapFrame *)malloc( 2 * SysPara.Map.x_max * SysPara.Map.y_max * sizeof(MapFrame));
    CrossRoad = (Cross *)malloc( SysPara.Map.crossnum * sizeof(Cross));
    ret = RoadMapInit(MapBlock, CrossRoad, &SysPara);
    if(ret) {
        printf("RoadMapInit Error!\n");
        return -1;
    }

    /* ����ͷ�ڵ��ʼ�� */
    CameraNode = (Node *)malloc( 3 * SysPara.Node.nodenum * sizeof(Node));  //��ֹ�ڴ治����
    ret = CameraInit( CameraNode, CrossRoad, &SysPara );
    if(ret) {
        printf("CameraInit Error!\n");
        return -2;
    }

    /* Ŀ���ʼ�� */
    TrackObject = (Object *)malloc( 3 * SysPara.Object.objectnum * sizeof(Object));
    ret = ObjectInit( TrackObject, &SysPara );
    if(ret) {
        printf("ObjectInit Error!\n");
        return -3;
    }
    printf("Init Success!\n");

    /* ����ͷ�ڵ��ܿ� */
    CameraControl( CameraNode, TrackObject, &SysPara );
    sleep(1);

    /* Ŀ���˶��ܿ� */
    ObjectMovement( MapBlock, CrossRoad, TrackObject, &SysPara );
    sleep(1);

    /* ����ܿ� */
    OutputControl( CameraNode, TrackObject, &SysPara );
    sleep(1);

    /* main���̵߳ȴ� */
    while(1)
    {
        sleep(1);
    }

    return 0;
}
