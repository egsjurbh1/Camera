#ifndef _MAP_H
#define _MAP_H

#include "system.h"
/* ��ͼ��� */
#define IMPASSABLE      0           //����ͨ��
#define PASSABLE        1           //��ͨ��
#define PASSED          2           //���߹���·

/* ��λ */
#define NORTH           1       //��
#define SOUTH           2       //��
#define EAST            3       //��
#define WEST            4       //��

/* ���� */
typedef struct MapCooStruct
{
    int x;
    int y;
}MapCoo;

/* ��ͼ */
typedef struct MapFrameStruct
{
    int abletag;
    MapCoo coo;
}MapFrame;

/* ����·����Ϣ */
typedef struct CrossStruct
{
    int roadid;         //·�ڱ��
    MapCoo coo;         //·����������
    int halfwidth;      //�������
    int halflength;     //�����볤
}Cross;

extern int RoadMapInit(MapFrame *mapf, Cross *cross, SystemPara *sys); //��·��ͼ��ʼ������

#endif // _MAP_H
