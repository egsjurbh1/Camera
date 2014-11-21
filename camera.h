#ifndef _CAMERA_H
#define _CAMERA_H

#define NODENUM 5           //�ڵ���
/* ��ϵǿ�� */
#define STRENGTHINIT 1.3    //strength��ֵ
#define STRENGTHINCRE 1.0   //����
#define STRENGTHRATIO 0.1   //ϵ��
#define PVALUE 0.01         //��ʽ�������ʦ�
#define TVALUE 1.0          //��ʽ������ֵ

/* ��Ϣ���������� */
#define NONEA 0             //��ʼ��
#define SUCCESSA 1          //�ƽ��ɹ�
#define FAILA 2             //�����ڽڵ�
#define TRACKTASK 11        //��������
#define MULTICAST 1         //���񷢲�ģʽ���鲥
#define BROADCAST 2         //���񷢲�ģʽ���㲥

/* ��� */
#define NONEID 0            //�ޱ��
#define NODEID 101          //�ڵ���ʼ���

#include "map.h"
#include "object.h"

/* ������Ϣ */
typedef struct TaskInfoStruct
{
    int nodeid;     //��ϢԴ�ڵ�
    int objectid;   //Ŀ��
    int tasktype;   //��Ϣ���� NONEA��������TRACKTASK����������
}TaskInfo;

/* �ڵ���Ϣ */
typedef struct NodeStruct
{
    int nodeid;                     //�ڵ���
    float rstrength[NODENUM];       //�ڵ��ϵǿ�ȱ�
    int SdetectO[OBJECTNUM][2];     //Ŀ����ټ�,λ��0��Ŀ���ţ�λ��1��Դ�ڵ��
    int StrackO[OBJECTNUM][2];      //��������
    MapCoo coo;
}Node;

/* �ڵ㷴����Ϣ�����̲߳����ṹ�� */
typedef struct CameraThreadArgStruct
{
    int **nodemsg;
    Node *node;
    Object *object;
    TaskInfo **taskmsg;
}CameraThreadArg;

extern int CameraInit( Node *node, int nodemsg[][NODENUM], TaskInfo taskmsg[][OBJECTNUM]);                      //�ڵ��ʼ������
extern void CameraControl( Node *node, int nodemsg[][NODENUM], TaskInfo taskmsg[][OBJECTNUM], Object *obj );    //�ڵ��ܿغ���
extern int count_communication(int flag);                                                                       //ͨ�Ŵ���ͳ��

#endif // _CAMERA_H
