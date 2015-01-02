#ifndef _OBJECT_H
#define _OBJECT_H

/* �˶�ģʽ */
#define PASSIVEMODE     1      //·������
#define ACTIVEMODE      2      //���Ѱ·
#define MAXROUTE        50     //���·���洢��
#define PERCENT         4      //1:4����,find_next�����������

#include "map.h"
#include "system.h"

/* Ŀ����Ϣ */
typedef struct ObjectStruct
{
    int objectid;               //Ŀ����
    int speed;                  //�˶��ٶ�
    int direction;              //�˶���λ����modeΪACTIVEEMODEʱ��Ч��
    int mode;                   //�˶�ģʽ
    float distance;             //�ͽڵ�ľ���
    MapCoo coo;                 //��ǰ����
    int route[MAXROUTE][2];     //·����Ϣ����modeΪPASSIVEEMODEʱ��Ч��
    int r_size;                 //·��size
}Object;

/* Ŀ���˶��̲߳����ṹ�� */
typedef struct MoveArgStruct
{
    MapFrame *mapf;
    Object *obj;
    Cross *cross;
    SystemPara *sys;
}MoveArg;

extern int ObjectInit( Object *obj, SystemPara *sys ); //Ŀ���ʼ������
extern void ObjectMovement( MapFrame *mapf, Cross *cross, Object *obj, SystemPara *sys );   //Ŀ���˶����ƺ���

#endif // _OBJECT_H
