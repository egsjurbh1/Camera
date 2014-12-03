#ifndef _CAMERA_H
#define _CAMERA_H

#define MAXNODE         20      //node max
#define MAXOBJECT       40      //object max

/* ͨ�� */
#define TASKCOMMU       1       //���񷢲�ͨ��
#define FEEDBACKCOMMU   2       //������Ϣͨ��
#define OUTPUTIC        3       //���˲ʱͨ����

/* ��Ϣ���������� */
#define NONEA           0       //��ʼ��
#define SUCCESSA        1       //�ƽ��ɹ�
#define FAILA           2       //�����ڽڵ�

#define TRACKTASK       1       //��������


#include "map.h"
#include "object.h"
#include "system.h"

/* ������Ϣ */
typedef struct TaskInfoStruct
{
    int nodeid;         //��ϢԴ�ڵ�
    int objectid;       //Ŀ��
    int tasktype;       //��Ϣ���� NONEA��������TRACKTASK����������
}TaskInfo;

/* ���񼯺� */
typedef struct TaskSetStruct
{
    int objectid;                   //Ŀ����
    int postnodeid;                 //��ϢԴ�ڵ���
}TaskSet;

/* �ڵ���Ϣ */
typedef struct NodeStruct
{
    int nodeid;                     //�ڵ���
    MapCoo coo;                     //�ڵ�λ������
    int fovlength;                  //���򳤶�
    int fovhalfwidth;               //������
    int direction;                  //��ط���
    int commu_mode;                 //��������ķ�ʽ MULTICAST,BROADCAST
    float load;                     //�ڵ㸺��
    float utility;                  //�ڵ�Ч��
    float *rstrength;               //�ڵ��ϵǿ�ȱ�
    TaskSet *SdetectO;              //Ŀ����ټ�
    TaskSet *StrackO;               //��������
}Node;

/* �ڵ㷴����Ϣ�����̲߳����ṹ�� */
typedef struct CameraThreadArgStruct
{
    Node *node;
    Object *object;
    SystemPara *sys;
}CameraThreadArg;

extern int CameraInit(Node *node, Cross *cross, SystemPara *sys);                                       //�ڵ��ʼ������
extern void CameraControl( Node *node, Object *obj, SystemPara *sys );                                  //�ڵ��ܿغ���
extern float count_communication(int flag, int type, float *ic, float taskcost, float feedbackcost);    //ͨ�Ŵ���ͳ��

#endif // _CAMERA_H
