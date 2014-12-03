#ifndef _OBJECT_H
#define _OBJECT_H

/* �˶�ģʽ */
#define PASSIVEMODE     1      //·������
#define ACTIVEMODE      2      //���Ѱ·

#define PERCENT         4      //1:4����,find_next�����������

/* ·��ģʽ */
#define RMODE_A         1      //ѭ���ƶ�·��
#define RMODE_B         2
#define RMODE_C         3

#include "map.h"
#include "system.h"

/* Ŀ����Ϣ */
typedef struct ObjectStruct
{
    int objectid;   //Ŀ����
    int speed;      //�˶��ٶ�
    int direction;  //�˶���λ����modeΪACTIVEEMODEʱ��Ч��
    int mode;       //�˶�ģʽ
    int route;      //·��ѡ����modeΪPASSIVEEMODEʱ��Ч��
    float distance; //�ͽڵ�ľ���
    MapCoo coo;     //��ǰ����
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
