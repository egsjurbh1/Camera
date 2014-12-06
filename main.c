/** \brief Camera Networks Software Framework
 *
 * \author chinglee
 * \update 141204
 * \version v1.2.0
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
    SystemPara SysPara;                //ϵͳ����
    MapFrame *MapBlock;                //��ͼ
    Cross *CrossRoad;                  //·��
    Object *TrackObject;               //Ŀ��
    Node *CameraNode;                  //�ڵ�

    /* ϵͳ������ʼ�� */
    ret = SystemInit(&SysPara);
    if(ret) {
        printf("SystemInit Error!\n");
        return -9;
    }

    /* �����ڴ�ռ� */
    MapBlock = (MapFrame *)malloc( 2 * SysPara.Map.x_max * SysPara.Map.y_max * sizeof(MapFrame));
    CrossRoad = (Cross *)malloc( SysPara.Map.crossnum * sizeof(Cross));
    CameraNode = (Node *)malloc( 3 * SysPara.Node.nodenum * sizeof(Node));  //��ֹ�ڴ治����
    TrackObject = (Object *)malloc( 3 * SysPara.Object.objectnum * sizeof(Object));

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

    /* ���̵߳ȴ� */
    while(1)
    {
        sleep(1);
    }

    return 0;
}
