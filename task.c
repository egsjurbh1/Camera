/** \brief �������
 *
 * \author lq
 * \update 141224
 * \return
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <math.h>
#include <windows.h>
#include "task.h"

static int TAmethod_TQTA( Task *task, Node *node, int tasknum, int nodenum, int nodeid_init, int taskid_init);
static void related_groups_generation( Task *task, Node *node, int tasknum, int nodenum );
static int isQosSatisfied( Node *node, Task *task, Task *tasktest, int taskid_init );
static void addtasktonode(Node *node, Task *task);

/***************************************************************
*   �����ʼ������
*****/
int TaskInit( Task *task, SystemPara *sys )
{
    int i;
    int tasknum, taskid_init;

    taskid_init = sys->Task.taskid_init;
    tasknum = sys->Task.tasknum;

    //��ʼ������
    for( i = 0; i < tasknum; ++i )
    {
        task[i].id = taskid_init + i;
        task[i].execv_node = NONEID;
        task[i].state = STANDING;
        task[i].trans_flag = 0;
        task[i].relatedgroup = (int *)malloc(tasknum * sizeof(int));
    }
    //�����������
    task[0].coo.x = 15;
    task[0].coo.y = 25;
    task[0].qos_para = 0.9;
    task[0].res_cost = 0.1;

    task[1].coo.x = 15;
    task[1].coo.y = 25;
    task[1].qos_para = 0.9;
    task[1].res_cost = 0.2;

    task[2].coo.x = 25;
    task[2].coo.y = 15;
    task[2].qos_para = 0.9;
    task[2].res_cost = 0.3;

    task[3].coo.x = 25;
    task[3].coo.y = 15;
    task[3].qos_para = 0.9;
    task[3].res_cost = 0.3;

    task[4].coo.x = 35;
    task[4].coo.y = 25;
    task[4].qos_para = 0.85;
    task[4].res_cost = 0.5;

    task[5].coo.x = 35;
    task[5].coo.y = 25;
    task[5].qos_para = 0.9;
    task[5].res_cost = 0.4;

    task[6].coo.x = 35;
    task[6].coo.y = 25;
    task[6].qos_para = 0.95;
    task[6].res_cost = 0.3;

    task[7].coo.x = 25;
    task[7].coo.y = 35;
    task[7].qos_para = 0.95;
    task[7].res_cost = 0.3;

    return 0;
}

/******************************************************************
*   ��������ܿ�
*   ���룺������Ϣ, �ڵ�״̬, ���Ʋ���
*   �����������������߳�
****/
void TaskAllocation( Task *task, Node *node, SystemPara *sys )
{
    int ret, methodtype, tasknum, nodenum, nodeid_init, taskid_init;

    methodtype = sys->Task.method_type;
    tasknum = sys->Task.tasknum;
    nodenum = sys->Node.nodenum;
    nodeid_init = sys->Node.nodeid_init;
    taskid_init = sys->Task.taskid_init;

    switch(methodtype)
    {
    case METHOD_TQTA:   /* TQTA, Tranfer based QoS-aware Task Allocation ���� */
        ret = TAmethod_TQTA( task, node, tasknum, nodenum, nodeid_init, taskid_init );
        if(ret != 0) {
            printf("Running TAmethod TQTA error!\n");
            exit(1);
        }
        break;
    default:
        break;
    }


}

/*************************************
*   TQTA, Tranfer based QoS-aware Task Allocation ����
*************************************/
static int TAmethod_TQTA( Task *task, Node *node, int tasknum, int nodenum, int nodeid_init, int taskid_init)
{
    int i,j,knode,jnode;
    int *reg;
    float minres,maxv;
    //������������ɣ������������
    related_groups_generation(task, node, tasknum, nodenum);
    for(i = 0; i < tasknum; ++i)
    {
        //�ҵ�������и�����С�Ľڵ�
        reg = task[i].relatedgroup;
        //����������飬�����޷�����
        if(*reg == 0) {
            task[i].state = FAILEDT;
            continue;
        }

        knode = *reg - nodeid_init;
        minres = node[knode].load;
        while(*reg != 0)
        {
            if(node[*reg - nodeid_init].load < minres) {
                knode = *reg - nodeid_init;
                minres = node[knode].load;
            }
            ++reg;
        }

        //�ж�QoS
        if(isQosSatisfied( &node[knode], task, &task[i], taskid_init) )
        {
            addtasktonode( &node[knode], &task[i] );//�������ڵ�����
        }
        else
        {
            //����ת�Ʋ���
            if(node[0].load != 0)
                maxv = 1 / node[0].load;
            else
                maxv = MAXVALUE;
            jnode = 0;
            for(j = 0; j < nodenum; ++j)
            {
                if(j == knode)
                    continue;
                if(node[j].load == 0)
                {
                    jnode = j;
                    maxv = MAXVALUE;
                }
                else
                {
                    if(1 / node[j].load > maxv) {
                        maxv = 1 / node[j].load;
                        jnode = j;
                    }
                }
            }
            //�ж�QoS
            if(isQosSatisfied(&node[jnode], task, &task[i], taskid_init))
            {
                addtasktonode( &node[jnode], &task[i] );
                task[i].trans_flag = 1;
            }
            else
            {
                task[i].state = FAILEDT;
            }
        }
    }
    return 0;
}

/**< ������������ɣ������������ */
static void related_groups_generation( Task *task, Node *node, int tasknum, int nodenum )
{
    int i,j,k;
    Task tmp;

    //���������
    for(i = 0; i < tasknum; ++i)
    {
        k = 0;
        for(j = 0; j < nodenum; ++j)
        {
            if(task[i].coo.y == node[j].coo.y && task[i].coo.x == node[j].coo.x)
            {
               task[i].relatedgroup[k] = node[j].nodeid;
               ++k;
            }
        }
        task[i].relatedgroup[k] = 0;    //�������
    }

    //����(Bubble Sort)�����������
    for(i =0 ; i< tasknum-1; ++i)
    {
        for(j = 0; j < tasknum-i-1; ++j)
        {
            if(task[j].res_cost > task[j+1].res_cost)
            {
                tmp = task[j] ;
                task[j] = task[j+1] ;
                task[j+1] = tmp;
            }
        }
    }
}

/**< �ж�QoS */
static int isQosSatisfied( Node *node, Task *task, Task *tasktest, int taskid_init )
{
    int *taskn;
    float minqos, load;

    taskn = node->taskid;
    load = node->load;
    //�Ӹ���
    load += tasktest->res_cost;

    //�ڵ�������������
    if(node->tasknum == 0)
        return 1;
    //����������С��qos
    minqos = task[*taskn - taskid_init].qos_para;
    while(*taskn != 0)
    {
        if(task[*taskn - taskid_init].qos_para < minqos)
            minqos = task[*taskn - taskid_init].qos_para;
        ++taskn;
    }
    //�ж�QoS
    if(load <= minqos)
        return 1;
    else
        return 0;
}

/**< �������ڵ����� */
static void addtasktonode(Node *node, Task *task)
{
    int n = node->tasknum;
    //�ڵ����񼯸���
    node->taskid[n] = task->id;
    node->load += task->res_cost;
    ++node->tasknum;
    node->taskid[n+1] = 0;  //��β���
    //����״̬����
    task->execv_node = node->nodeid;
    task->state = RUNNING;
}



