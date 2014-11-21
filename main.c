/** \brief ����ͷ����Ŀ����ٷ���
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

    /*MapBlock[i][j]�洢��ͼ���꣨i,j��*/
    volatile int MapBlock[MAPX][MAPY];    //���� IMPASSABLE������ͨ�У�PASSABLE����ͨ��
    /*crossroad[i]�洢����·��i��Ϣ*/
    volatile Cross CrossRoad[CROSSNUM];
    volatile Object TrackObject[OBJECTNUM];   //����Ŀ��
    volatile Node CameraNode[NODENUM];  //����ͷ�ڵ�
    /*NodeMsg[i][j]�洢�ڵ�j�����ڵ�i����Ϣ*/
    volatile int NodeMsg[NODENUM][NODENUM];   //��Ϣ���� NONEACTION���ޣ�SUCCESSA���ƽ��ɹ���FAILA�������ڽڵ㣻
    /*TaskMsg[i][j]�洢�ڵ�i�յ�Ŀ��j��������Ϣ*/
    volatile TaskInfo TaskMsg[NODENUM][OBJECTNUM];

    /* ��·�͵�ͼ���ó�ʼ�� */
    ret = RoadMapInit(MapBlock, CrossRoad);
    if(ret != 0) {
        printf("RoadMapInit Error!\n");
        return -2;
    }

    /* ����ͷ�ڵ��ʼ�� */
    ret = CameraInit( CameraNode, NodeMsg, TaskMsg );
    if(ret != 0) {
        printf("CameraInit Error!\n");
        return -3;
    }

    /* Ŀ���ʼ�� */
    ret = ObjectInit( TrackObject );
    if(ret != 0) {
        printf("ObjectInit Error!\n");
        return -4;
    }
    printf("Init Success!\n");

    /* ����ͷ�ڵ��ܿ� */
    CameraControl( CameraNode, NodeMsg, TaskMsg, TrackObject );
    sleep(1);

    /* Ŀ���˶��ܿ� */
    ObjectMovement( MapBlock, TrackObject, EXECUTEACTION );
    sleep(1);

    /* �������ܿ� */
    OutputControl( CameraNode );
    sleep(1);

    /* main���̵߳ȴ� */
    while(1)
    {
        sleep(1);
    }

    return 0;
}
