#ifndef _TASK_H
#define _TASK_H

#define FAILTON     3       //���䵫�޷�ִ��
#define FAILEDT     2       //����ʧ��
#define RUNNING     1       //����ɹ�
#define STANDING    0       //δ����

#define MAXVALUE    10000   //Value����ֵ
#include "map.h"
#include "camera.h"

/* ������Ϣ */
typedef struct TaskStruct
{
    int id;             //����ID
    int state;          //�������״̬
    int trans_flag;     //����ת�Ʊ��
    int execv_node;     //���������ڵ�
    float res_cost;     //������Դ����
    float qos_para;     //����QoS��ֵ
    MapCoo coo;         //����λ��
    int *relatedgroup;  //���������
}Task;

extern int TaskInit( Task *task, SystemPara *sys );
extern void TaskAllocation( Task *task, Node *node, SystemPara *sys );     //��������ܿ�

#endif // _TASK_H
