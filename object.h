#ifndef _OBJECT_H
#define _OBJECT_H

#define OBJECTNUM 2 //Ŀ����
#define OBJECTID 601 //Ŀ����ʼ���
#define NONEOID 0 //�ޱ��

/* �˶����� */
#define STOPACTION   0  //ֹͣ
#define EXECUTEACTION 1     //ִ��
/* �˶�ģʽ */
#define PASSIVEMODE  11   //·������
#define ACTIVEMODE   12   //���Ѱ·
#define PERCENT 4         ////1:4����,find_next�����������

#include "map.h"

/* Ŀ����Ϣ */
typedef struct ObjectStruct
{
    int objectid;   //Ŀ����
    float speed;    //�˶��ٶ�
    int mode;       //�˶�ģʽ
    MapCoo coo;     //��ǰ����
    MapCoo destcoo; //Ŀ�ĵ�����
}Object;

/* Ŀ���˶��̲߳����ṹ�� */
typedef struct MoveArgStruct
{
    int **mapblock;
    Object *obj;
}MoveArg;

extern int ObjectInit( Object *obj ); //Ŀ���ʼ������
extern void ObjectMovement( int mapb[][MAPY], Object *obj, int configarg );   //Ŀ���˶����ƺ���

#endif // _OBJECT_H
