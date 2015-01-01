/** \brief �������
 *
 * \author lq
 * \update 150101
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
static int TAmethod_QTA( Task *task, Node *node, int tasknum, int nodenum, int nodeid_init, int taskid_init);
static int TAmethod_TA( Task *task, Node *node, int tasknum, int nodenum, int nodeid_init, int taskid_init);
static void related_groups_generation( Task *task, Node *node, int tasknum, int nodenum );
static int isQosSatisfied( Node *node, Task *task, Task *tasktest, int taskid_init );
static void addtasktonode(Node *node, Task *task);
static void setQoS(Task *task, float qos_para);
static void setTask(SystemPara *sys, Task *task, int coox, int cooy, float res_cost);
static void setTaskArray(int arraynum, SystemPara *sys, Task *task, int coox, int cooy, float *res_cost);

/***************************************************************
*   �����ʼ������
*****/
int TaskInit( Task *task, SystemPara *sys )
{
    //float rescostarray[10];
    int sysmode = sys->system_mode;

    if(sysmode != TASK_MODE)
        return 0;

    /* ����������� */
    ///·��һ(15,25)
    float rescostarray1[] = {0.06, 0.2, 0.5};
    setTaskArray(3, sys, task, 15, 25, rescostarray1);
    ///·��һ(25,15)
    float rescostarray2[] = {0.03, 0.3, 0.5};
    setTaskArray(3, sys, task, 25, 15, rescostarray2);
    ///·��һ(25,35)
    float rescostarray3[] = {0.05, 0.25, 0.5};
    setTaskArray(1, sys, task, 25, 35, rescostarray3);
    ///·��һ(35,25)
    float rescostarray4[] = {0.06, 0.25, 0.5};
    setTaskArray(1, sys, task, 35, 25, rescostarray4);
    ///·�ڶ�(65,25)
    float rescostarray5[] = {0.06, 0.25, 0.5};
    setTaskArray(1, sys, task, 65, 25, rescostarray5);
    ///·�ڶ�(75,15)
    float rescostarray6[] = {0.06, 0.25, 0.5};
    setTaskArray(1, sys, task, 75, 15, rescostarray6);
    ///·�ڶ�(85,25)
    float rescostarray7[] = {0.06, 0.25, 0.5};
    setTaskArray(1, sys, task, 85, 25, rescostarray7);
    ///·�ڶ�(75,35)
    float rescostarray8[] = {0.05, 0.24, 0.5};
    setTaskArray(1, sys, task, 75, 35, rescostarray8);

    return 0;
}

/******************************************************************
*   ��������ܿ�
*   ���룺������Ϣ, �ڵ�״̬, ���Ʋ���
*   �����������������߳�
****/
void TaskAllocation( Task *task, Node *node, SystemPara *sys )
{
    int ret, methodtype, tasknum, nodenum, nodeid_init, taskid_init, sysmode;

    methodtype = sys->Task.method_type;
    tasknum = sys->Task.tasknum;
    nodenum = sys->Node.nodenum;
    nodeid_init = sys->Node.nodeid_init;
    taskid_init = sys->Task.taskid_init;
    sysmode = sys->system_mode;

    if(sysmode != TASK_MODE)
        return;

    switch(methodtype)
    {
    case METHOD_TQTA:   /* TQTA, Tranfer based QoS-aware Task Allocation ���� */
        ret = TAmethod_TQTA( task, node, tasknum, nodenum, nodeid_init, taskid_init );
        if(ret != 0) {
            printf("Running TAmethod TQTA error!\n");
            exit(1);
        }
        break;
     case METHOD_QTA:   /* QTA, QoS-aware Task Allocation ���� */
        ret = TAmethod_QTA( task, node, tasknum, nodenum, nodeid_init, taskid_init );
        if(ret != 0) {
            printf("Running TAmethod QTA error!\n");
            exit(1);
        }
        break;
     case METHOD_TA:   /* TA, Task Allocation ���� */
        ret = TAmethod_TA( task, node, tasknum, nodenum, nodeid_init, taskid_init );
        if(ret != 0) {
            printf("Running TAmethod TA error!\n");
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

/*************************************
*   QTA, QoS-aware Task Allocation ����
*************************************/
static int TAmethod_QTA( Task *task, Node *node, int tasknum, int nodenum, int nodeid_init, int taskid_init)
{
    int i,knode;
    int *reg;
    float minres;
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
            task[i].state = FAILEDT;
        }
    }

    return 0;
}

/*************************************
*   TA, Task Allocation ����
*************************************/
static int TAmethod_TA( Task *task, Node *node, int tasknum, int nodenum, int nodeid_init, int taskid_init)
{
    int i,knode;
    int *reg;
    float minres;
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

        //�жϷ�������
        if( (1 - node[knode].load) >= task[i].res_cost )
        {
            addtasktonode( &node[knode], &task[i] );//�������ڵ�����
        }
        else
        {
            task[i].state = FAILEDT;
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

/**< Set QoS by res_cost */
static void setQoS(Task *task, float qos_para)
{
    float res = task->res_cost;

    if(res > 0 && res < 0.1)
        task->qos_para = qos_para;
    else if(res >= 0.1 && res < 0.5)
        task->qos_para = qos_para;
    else if(res >= 0.5)
        task->qos_para = qos_para;
    else
        printf("Set QoS error.\n");
}

/**< Set Task */
static void setTask(SystemPara *sys, Task *task, int coox, int cooy, float res_cost)
{
    int tasknum, taskid_init;
    float qos_para;
    static int n = 0;

    taskid_init = sys->Task.taskid_init;
    tasknum = sys->Task.tasknum;
    qos_para = sys->Task.qos_para;

    if(n > tasknum){
        printf("!WARNING:Task Number Extra.\n");
        return;
    }

    task[n].coo.x = coox;
    task[n].coo.y = cooy;
    task[n].res_cost = res_cost;
    setQoS(&task[n], qos_para);
    task[n].id = taskid_init + n;
    task[n].execv_node = NONEID;
    task[n].state = STANDING;
    task[n].trans_flag = 0;
    task[n].relatedgroup = (int *)malloc(tasknum * sizeof(int));
    printf("Task:%d Initialize Success.\n", task[n].id );

    ++n;
}

/**< Set Task Array */
static void setTaskArray(int arraynum, SystemPara *sys, Task *task, int coox, int cooy, float *res_cost)
{
    int i;

    for(i = 0; i < (3*arraynum); ++i)
    {
        switch(i%3)
        {
        case 0:
            setTask(sys, task, coox, cooy, res_cost[0]);
            break;
        case 1:
            setTask(sys, task, coox, cooy, res_cost[1]);
            break;
        case 2:
            setTask(sys, task, coox, cooy, res_cost[2]);
            break;
        default:
            break;
        }
    }
}
