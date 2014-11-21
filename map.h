#ifndef _MAP_H
#define _MAP_H

#define PASSED 2        //���߹���·
#define PASSABLE 1      //��ͨ��
#define IMPASSABLE 0    //����ͨ��
#define ENDROUTE 9999   //·���߽�
/* ��ʼ������ */
#define CROSSNUM 2      //����·����
#define CROSS1_X 1      //·��һ��������
#define CROSS1_Y 1
#define CROSS2_X 4      //·�ڶ���������
#define CROSS2_Y 1
#define MAPX 10         //��ͼx��������
#define MAPY 10         //��ͼy��������
#define ROADID 901      //·����ʼ���

/* ���� */
typedef struct MapCooStruct
{
    int x;  //x��
    int y;
}MapCoo;

/* ����·����Ϣ */
typedef struct CrossStruct
{
    int roadid; //·�ڱ��
    MapCoo coo;
}Cross;

extern int RoadMapInit(int mapblock[][MAPY], Cross *cross); //��·��ͼ��ʼ������

#endif // _MAP_H
