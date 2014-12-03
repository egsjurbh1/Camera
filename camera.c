/** \brief ����ͷ�ڵ�
 *
 * \author lq
 * \update 141202
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
#include "camera.h"

static void NodeMsgThread(void *arg);       //�ڵ㷴����Ϣ�����߳�
static void NodeTaskThread(void *arg);      //�ڵ�������Ϣ�����߳�
static void ObjcetDetectThread(void *arg);  //�ڵ�Ŀ�����߳�

static void check_nodemsg(Node *node, SystemPara *sys);
static void check_taskmsg( Node *node, SystemPara *sys);
static void ObjcetDetect( Node *node, Object *obj, SystemPara *sys );
static void post_task( Node *node, int i, SystemPara *sys );
static int isinfov( Node *node, Object *obj);
static int countUtility( Node *node, Object *obj, float *obju );
float count_communication(int flag, int type, float *ic, float taskcost, float feedbackcost);

/**< �߳�ͨ��ȫ���� */
/*NodeMsg[i][j]�洢�ڵ�j�����ڵ�i����Ϣ*/
static volatile int NodeMsg[MAXNODE][MAXNODE];
/*TaskMsg[i][j]�洢�ڵ�i�յ�Ŀ��j��������Ϣ*/
static volatile TaskInfo TaskMsg[MAXNODE][MAXOBJECT];

/***********************************************************
*   ����ͷ�ڵ��ʼ������
*   input:��Ϣ�������ڵ�
*****/
int CameraInit(Node *node, Cross *cross, SystemPara *sys)
{
    int i,j;
    int nodenum,objectnum,commu_mode,fovhalfwidth,fovlength,firstnodeid;
    float strengthinit;

    /* ȡϵͳ���� */
    nodenum = sys->Node.nodenum;
    objectnum = sys->Object.objectnum;
    commu_mode = sys->Node.commu_mode;
    fovhalfwidth = sys->Node.fovhalfwidth;
    fovlength = sys->Node.fovlength;
    firstnodeid = sys->Node.nodeid_init;
    strengthinit = sys->Node.strengthinit;
    /* NodeMsg��Ϣ��ʼ�� */
    for( i = 0; i < nodenum; i++)
        for( j = 0; j < nodenum; j++)
            NodeMsg[i][j] = NONEA;
    /* TaskMsg��Ϣ��ʼ�� */
    for( i = 0; i < objectnum; i++)
        for( j = 0; j < nodenum; j++) {
            TaskMsg[i][j].tasktype = NONEA;
            TaskMsg[i][j].nodeid = NONEID;
        }

    /* ����ͷ�ڵ������ʼ��*/
    for( i = 0; i < nodenum; i++, firstnodeid++) {
        node[i].nodeid = firstnodeid;   //�ڵ�ID,����Ϊ1
        node[i].commu_mode = commu_mode;
        node[i].load = 0;
        node[i].utility = 0;
        node[i].fovhalfwidth = fovhalfwidth;
        node[i].fovlength = fovlength;

        //�ڽӱ��ϵǿ��
        node[i].rstrength = (float *)malloc(nodenum * sizeof(float));
        for( j = 0; j < nodenum; j++)
            node[i].rstrength[j] = strengthinit;
        //Ŀ�꼯
        node[i].SdetectO = (TaskSet *)malloc( objectnum * sizeof(TaskSet));
        node[i].StrackO = (TaskSet *)malloc( objectnum * sizeof(TaskSet));
        for( j = 0; j < objectnum; j++) {
            node[i].SdetectO[j].objectid = NONEID;
            node[i].SdetectO[j].postnodeid = NONEID;
            node[i].StrackO[j].objectid = NONEID;
            node[i].StrackO[j].postnodeid = NONEID;
        }
    }

    /* ����ͷ�ڵ�λ�ó�ʼ��*/
    //�ڵ�C1(·��2 ��)
    node[0].coo.x = cross[1].coo.x - cross[1].halfwidth;
    node[0].coo.y = cross[1].coo.y;
    node[0].direction = WEST;
    //�ڵ�C2(·��1 ��)
    node[1].coo.x = cross[0].coo.x + cross[0].halfwidth;
    node[1].coo.y = cross[0].coo.y;
    node[1].direction = EAST;
    //�ڵ�C3(·��1 ��)
    node[2].coo.x = cross[0].coo.x;
    node[2].coo.y = cross[0].coo.y + cross[0].halfwidth;
    node[2].direction = NORTH;
    //�ڵ�C4(·��1 ��)
    node[3].coo.x = cross[0].coo.x - cross[0].halfwidth;
    node[3].coo.y = cross[0].coo.y;
    node[3].direction = WEST;
    //�ڵ�C5(·��1 ��)
    node[4].coo.x = cross[0].coo.x;
    node[4].coo.y = cross[0].coo.y - cross[0].halfwidth;
    node[4].direction = SOUTH;
    //�ڵ�C6(·��2 ��)
    node[5].coo.x = cross[1].coo.x;
    node[5].coo.y = cross[1].coo.y + cross[1].halfwidth;
    node[5].direction = NORTH;
    //�ڵ�C7(·��2 ��)
    node[6].coo.x = cross[1].coo.x + cross[1].halfwidth;
    node[6].coo.y = cross[1].coo.y;
    node[6].direction = EAST;
    //�ڵ�C8(·��2 ��)
    node[7].coo.x = cross[1].coo.x;
    node[7].coo.y = cross[1].coo.y - cross[1].halfwidth;
    node[7].direction = SOUTH;
    printf("Camera Node Initialize Success.\n");
    return 0;
}

/************************************************************************
*   ����ͷ�ڵ��ܿ�
*   ���룺�ڵ㣬Ŀ����Ϣ,��Ϣ
*   �����ڵ��߳�
****/
void CameraControl( Node *node, Object *obj, SystemPara *sys )
{
    int i, ret, nodenum;
    nodenum = sys->Node.nodenum;

    CameraThreadArg nodemsgarg[nodenum], nodetaskarg[nodenum], objectdectart[nodenum];
    pthread_t nodemsgt[nodenum], nodetaskt[nodenum], objectdectt[nodenum];
    pthread_attr_t attr;

    //init
    pthread_attr_init(&attr);
    // explicitly specify as joinable PTHREAD_CREATE_JOINABLE or detached  PTHREAD_CREATE_DETACHED
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    /* �̲߳����ӿ� */
    for( i = 0; i < nodenum; i++ )
    {
        nodemsgarg[i].node = &node[i];
        nodemsgarg[i].sys = sys;
        nodetaskarg[i].node = &node[i];
        nodetaskarg[i].sys = sys;
        objectdectart[i].node = &node[i];
        objectdectart[i].object = obj;
        objectdectart[i].sys = sys;
    }

    /* ���������ڵ��߳� */
    for( i = 0; i < nodenum; i++ )
    {
        //NodeMsgThread�߳�
        ret = pthread_create(&nodemsgt[i], &attr, (void *)NodeMsgThread, &nodemsgarg[i]);
        if(ret!=0) {
            printf("Create NodeMsgThread %d error!\n", i);
            exit(1);
        }
        sleep(1);
        //NodeTaskThread�߳�
        ret = pthread_create(&nodetaskt[i], &attr, (void *)NodeTaskThread, &nodetaskarg[i]);
        if(ret!=0) {
            printf("Create NodeTaskThread %d error!\n", i);
            exit(1);
        }
        sleep(1);
        //ObjcetDetectThread�߳�
        ret = pthread_create(&objectdectt[i], &attr, (void *)ObjcetDetectThread, &objectdectart[i]);
        if(ret!=0) {
            printf("Create ObjcetDetectThread %d error!\n", i);
            exit(1);
        }
        sleep(1);
        printf("Node:%d Thread Created.\n", node[i].nodeid );
    }
    //release thread attribute
    pthread_attr_destroy(&attr);
}

/*************************************************************************************
*   �ڵ��߳�1���ڵ�䷴����Ϣ����
*
*****/
static void NodeMsgThread( void *arg )
{
    //�ӿڲ���
    CameraThreadArg *psarg;
    Node *node;
    SystemPara *sys;

    //��������
    psarg = (CameraThreadArg *)arg;
    node = psarg->node;
    sys = psarg->sys;

    //������Ϣ����
    check_nodemsg( node, sys );
}

/******������Ϣ����******/
static void check_nodemsg(Node *node, SystemPara *sys)
{
    int i,n,j,nodeid;
    int nodeid_init,nodenum;
    float strengthincre,strengthratio,maxsvalue,minsvalue;
    /* ȡϵͳ���� */
    nodeid_init = sys->Node.nodeid_init;
    nodenum = sys->Node.nodenum;
    strengthratio = sys->Node.strengthratio;
    strengthincre = sys->Node.strengthincre;
    maxsvalue = sys->Node.maxvalue;
    minsvalue = sys->Node.minvalue;

    nodeid = node->nodeid;
    n = nodeid - nodeid_init;//ȡ�ڵ�Ŷ�Ӧ����Ϣλ��

    while(1)
    {
        /* ɨ�跴����Ϣ�� */
        for(i = 0; i < nodenum; i++)
        {
            if(i == n)
                continue;

            /* �ж���Ϣ���� */
            if(NodeMsg[n][i] == SUCCESSA) {
                node->rstrength[i] += strengthincre; //�޸�ǿ��
                //printf("�ڵ�%d �� �ڵ�%d ��ϵ��ǿ��%.2f\n",(nodeid_init+i), nodeid, node->rstrength[i]);
            }
            else if(NodeMsg[n][i] == FAILA) {
                node->rstrength[i] -= (strengthincre * strengthratio); //�޸�ǿ��
                //printf("�ڵ�%d �� �ڵ�%d ��ϵ������%.2f\n",(nodeid_init+i), nodeid, node->rstrength[i]);
            }

            /* strengthֵ��Χ���� */
            if(node->rstrength[i] >= maxsvalue)
                node->rstrength[i] = maxsvalue;
            else if(node->rstrength[i] <= minsvalue)
                node->rstrength[i] = minsvalue;
            NodeMsg[n][i] = NONEA;    //��Ϣ��ʼ��
        }
        sleep(1);
    }

}

/*************************************************************************************************
*   �ڵ��߳�2:�ڵ�������Ϣ����
*
*******/
static void NodeTaskThread( void *arg )
{
    //�ӿڲ���
    CameraThreadArg *psarg;
    Node *node;
    SystemPara *sys;

    //��������
    psarg = (CameraThreadArg *)arg;
    node = psarg->node;
    sys = psarg->sys;

    //������Ϣ����
    check_taskmsg(node, sys);
}

/******������Ϣ����******/
static void check_taskmsg( Node *node, SystemPara *sys )
{
    int i,n,nodeid,m;
    float it,feedbackcost;
    int nodeid_init,objectnum;

    /* ȡϵͳ���� */
    nodeid_init = sys->Node.nodeid_init;
    objectnum = sys->Object.objectnum;
    feedbackcost = sys->Node.feedbackcost;

    nodeid = node->nodeid;
    n = nodeid - nodeid_init;    //ȡ�ڵ�Ŷ�Ӧ����Ϣλ��
    while(1)
    {
        /* ɨ��������Ϣ�� */
        for(i = 0; i < objectnum; i++)
        {
            if( TaskMsg[n][i].tasktype == TRACKTASK )
            {
                //Ŀ���Ѿ��ڸ��ټ���
                if( TaskMsg[n][i].objectid == node->StrackO[i].objectid )
                {
                    m = node->StrackO[i].postnodeid - nodeid_init;   //ȡ֮ǰԴ�ڵ�Ŷ�Ӧ����Ϣλ��
                    NodeMsg[m][n] = FAILA;  //n�ڵ㷢����Ϣ��m�ڵ�
                    count_communication(1,FEEDBACKCOMMU,&it,0,feedbackcost);
                }
                //���¸��ټ�
                node->StrackO[i].objectid = TaskMsg[n][i].objectid;
                node->StrackO[i].postnodeid = TaskMsg[n][i].nodeid;
                //printf("�ڵ� %d �������񼯸��£�����Ŀ�꣺%d �����ڵ� %d\n",nodeid, node->StrackO[i].objectid, node->StrackO[i].postnodeid);
            }
            //��Ϣ����ʼ��
            TaskMsg[n][i].nodeid = NONEID;
            TaskMsg[n][i].tasktype = NONEA;
        }
        sleep(1);
    }

}

/*************************************************************************************************
*   �ڵ��߳�3��Ŀ����
*******/
static void ObjcetDetectThread( void *arg )
{
    //�ӿڲ���
    CameraThreadArg *psarg;
    Node *node;
    Object *obj;
    SystemPara *sys;

    //��������
    psarg = (CameraThreadArg *)arg;
    node = psarg->node;
    obj = psarg->object;
    sys = psarg->sys;

    //Ŀ����
    ObjcetDetect( node, obj, sys );
}

/********�ڵ�Ŀ����*******/
static void ObjcetDetect( Node *node, Object *obj, SystemPara *sys )
{
    int i,n,nodeid,m;
    int nodeid_init,objectnum,maxnum;
    float maxload;
    /* ȡϵͳ���� */
    nodeid_init = sys->Node.nodeid_init;
    objectnum = sys->Object.objectnum;
    maxload = sys->Node.maxload;
    maxnum = sys->Node.maxnum;

    int tag[objectnum];
    float obju[objectnum];

    nodeid = node->nodeid;
    n = nodeid - nodeid_init;    //ȡ�ڵ�Ŷ�Ӧ����Ϣλ��

    for(i = 0; i < objectnum; i++)
        tag[i] = 0;

    while(1)
    {
        node->utility = 0; //���Ŀ��ǰ��Ч��ֵ��ʼ��
        for( i = 0; i < objectnum; i++ )
        {
            if( isinfov( node, &obj[i]) ) //Ŀ���ڽڵ�FOV��
            {
                //�ڵ㳬���أ�����Ŀ�����
                if( node->load >= maxload) {
                    node->load = maxload;
                    continue;
                }

                if(!tag[i]) {
                    node->load = (node->load * maxnum + 1) / maxnum;    //���½ڵ㸺��
                    printf("Object:%d in Node:%d FOV\n", obj[i].objectid, nodeid );
                    tag[i] = 1;
                }
                countUtility( node, &obj[i], &obju[i]); //����Ч��

                if( obj[i].objectid == node->SdetectO[i].objectid ) //Ŀ���Ѽ��뵽Ŀ�꼯
                    continue;
                else    //������
                {
                    if( obj[i].objectid == node->StrackO[i].objectid ) //Ŀ���ڸ��ټ���
                    {
                        /* Ŀ������⼯ */
                        node->SdetectO[i].objectid = obj[i].objectid;
                        node->SdetectO[i].postnodeid = nodeid;
                        /* ��Դ�ڵ㷴����Ϣ */
                        m = node->StrackO[i].postnodeid - nodeid_init;
                        NodeMsg[m][n] = SUCCESSA;
                        /* ����������Ϣ */
                        post_task( node, i, sys );
                    }
                    else    //�״μ�⵽Ŀ��
                    {
                        /* Ŀ������⼯ */
                        node->SdetectO[i].objectid = obj[i].objectid;
                        node->SdetectO[i].postnodeid = nodeid;
                        /* ����������Ϣ */
                        post_task( node, i, sys );
                    }
                }
            }
            else
            {
                if(tag[i])
                    node->load = (node->load * maxnum - 1) / maxnum;    //���½ڵ㸺��
                tag[i] = 0;
                node->SdetectO[i].objectid = NONEID; //��ʼ��
                node->SdetectO[i].postnodeid = NONEID;
                obj[i].distance = 0;
                obju[i] = 0;
            }
            node->utility += obju[i];   //�ڵ���Ч�ø���
        }
        sleep(1);
    }
}

/*****����������Ϣ*****/
static void post_task( Node *node, int i, SystemPara *sys )
{
    int j, n, nodeid, percent;
    int mode, nodenum, nodeid_init, objectid_init;
    float it, tvalue, pvalue, taskcost, feedbackcost;
    /* ȡϵͳ���� */
    nodeid_init = sys->Node.nodeid_init;
    objectid_init = sys->Object.objectid_init;
    tvalue = sys->Node.tvalue;
    pvalue = sys->Node.pvalue;
    nodenum = sys->Node.nodenum;
    taskcost = sys->Node.taskcost;
    feedbackcost = sys->Node.feedbackcost;

    nodeid = node->nodeid;
    mode = node->commu_mode;
    n = nodeid - nodeid_init;    //ȡ�ڵ�Ŷ�Ӧ����Ϣλ��

    for( j = 0; j < nodenum; j++ )
    {
        if( j == n)
            continue;
        switch(mode) {
        case MULTICAST:
            if( node->rstrength[j] >= tvalue ) //������ֵ
            {
                /* 100%����������Ϣ */
                TaskMsg[j][i].nodeid = nodeid;
                TaskMsg[j][i].tasktype = TRACKTASK;
                TaskMsg[j][i].objectid = i + objectid_init;
                count_communication(1,TASKCOMMU,&it,taskcost,feedbackcost);
                //printf("�ڵ� %d �� �ڵ� %d ���� Ŀ�� %d ��������\n", nodeid, j+NODEID, TaskMsg[j][i].objectid );
            }
            else {
                /* �����ʷ���������Ϣ */
                srand((unsigned)time(NULL));
                percent = 1 / pvalue;
                if( 1 == rand() % percent)
                {
                    TaskMsg[j][i].nodeid = nodeid;
                    TaskMsg[j][i].tasktype = TRACKTASK;
                    TaskMsg[j][i].objectid = i + objectid_init;
                    count_communication(1,TASKCOMMU,&it,taskcost,feedbackcost);
                    //printf("�ڵ� %d �� �ڵ� %d ���� Ŀ�� %d ��������\n", nodeid, j+NODEID, TaskMsg[j][i].objectid );
                }
            }
            break;

        case BROADCAST:
            /* 100%����������Ϣ */
            TaskMsg[j][i].nodeid = nodeid;
            TaskMsg[j][i].tasktype = TRACKTASK;
            TaskMsg[j][i].objectid = i + objectid_init;
            count_communication(1,TASKCOMMU,&it,taskcost,feedbackcost);
            //printf("�ڵ� %d �� �ڵ� %d ���� Ŀ�� %d ��������\n", nodeid, j+NODEID, TaskMsg[j][i].objectid );
            break;

        default:
            break;
        }
    }
}
/*****�ж�Ŀ���Ƿ���FOV��*****/
static int isinfov( Node *node, Object *obj)
{
    switch(node->direction){
    case NORTH:
        if(obj->coo.y >= node->coo.y && obj->coo.x <= (node->coo.y + node->fovlength) &&
           obj->coo.x <= (node->coo.x + node->fovhalfwidth) && obj->coo.x >= (node->coo.x - node->fovhalfwidth))
            return 1;
        else
            return 0;
        break;
    case SOUTH:
        if(obj->coo.y <= node->coo.y && obj->coo.y >= (node->coo.y - node->fovlength) &&
           obj->coo.x <= (node->coo.x + node->fovhalfwidth) && obj->coo.x >= (node->coo.x - node->fovhalfwidth))
            return 1;
        else
            return 0;
        break;
    case EAST:
        if(obj->coo.x >= node->coo.x && obj->coo.x <= (node->coo.x + node->fovlength) &&
           obj->coo.y <= (node->coo.y + node->fovhalfwidth) && obj->coo.y >= (node->coo.y - node->fovhalfwidth))
            return 1;
        else
            return 0;
        break;
    case WEST:
        if(obj->coo.x <= node->coo.x && obj->coo.x >= (node->coo.x - node->fovlength) &&
           obj->coo.y <= (node->coo.y + node->fovhalfwidth) && obj->coo.y >= (node->coo.y - node->fovhalfwidth))
            return 1;
        else
            return 0;
        break;
    default:
        break;
    }
    return 0;
}

/*****ͨ�Ŵ���ͳ��*****/
/**< ���룺flag 0 ����� 1 ����
*          type ͨ������ TASKCOMMU, FEEDBACKCOMMU
 */
float count_communication(int flag, int type, float *ic, float taskcost, float feedbackcost)
{
    static float n = 0;
    static float in = 0;

    if(flag == 1)
    {
        switch(type){
            case TASKCOMMU:
                n+=taskcost;
                in+=taskcost;
                break;
            case FEEDBACKCOMMU:
                n+=feedbackcost;
                in+=feedbackcost;
                break;
            default:
                break;
        }
    }
    else if(flag == OUTPUTIC)
    {
        *ic = in;
        in = 0; //˲ʱͳ��������
    }
    return n;
}

/*****Ŀ��˲ʱЧ�ü���*****/
static int countUtility( Node *node, Object *obj, float *obju )
{
    int ox,oy,nx,ny;
    float load,D;

    ox = obj->coo.x;
    oy = obj->coo.y;
    nx = node->coo.x;
    ny = node->coo.y;
    load = node->load;
    D = node->fovlength;

    //����Ŀ��ڵ����
    obj->distance = sqrt((ox - nx)*(ox - nx) + (oy - ny)*(oy - ny));
    //���㵱ǰЧ��
    *obju = (1 - load)*(1 - obj->distance / D);

    return 0;
}

