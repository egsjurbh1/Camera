/** \brief ����ͷ�ڵ�
 *
 * \author lq
 * \update 141108
 * \return
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "camera.h"

static void NodeMsgThread(void *arg);  //�ڵ㷴����Ϣ�����߳�
static void NodeTaskThread(void *arg); //�ڵ�������Ϣ�����߳�
static void ObjcetDetectThread(void *arg); //�ڵ�Ŀ�����߳�

static void check_nodemsg( Node *node, int nmsg[][NODENUM] );
static void check_taskmsg( Node *node, int nmsg[][NODENUM], TaskInfo tmsg[][OBJECTNUM] );
static void ObjcetDetect( Node *node, int nmsg[][NODENUM], TaskInfo tmsg[][OBJECTNUM], Object *obj );
static void post_task( TaskInfo tmsg[][OBJECTNUM], Node *node, int i);
int count_communication(int flag);

/***********************************************************
*   ����ͷ�ڵ��ʼ������
*   input:��Ϣ�������ڵ�
*****/
int CameraInit( Node *node, int nodemsg[][NODENUM], TaskInfo taskmsg[][OBJECTNUM])
{
    int i,j;
    int firstnodeid = NODEID;
    const int nodecoo[NODENUM][2] = {{3,1},{2,1},{1,2},{0,1},{1,0}};//�ڵ�����

    /* NodeMsg��Ϣ��ʼ�� */
    for( i = 0; i < NODENUM; i++)
        for( j = 0; j < NODENUM; j++)
            nodemsg[i][j] = NONEA;
    /* TaskMsg��Ϣ��ʼ�� */
    for( i = 0; i < OBJECTNUM; i++)
        for( j = 0; j < NODENUM; j++) {
            taskmsg[i][j].tasktype = NONEA;
            taskmsg[i][j].nodeid = NONEID;
        }

    /* ����ͷ�ڵ������ʼ��*/
    for( i = 0; i < NODENUM; i++, firstnodeid++) {
        //�ڵ�ID,����Ϊ1
        node[i].nodeid = firstnodeid;
        //�ڵ������ʼ��
        node[i].coo.x = nodecoo[i][0];
        node[i].coo.y = nodecoo[i][1];
        //�ڽӱ��ϵǿ��
        for( j = 0; j < NODENUM; j++)
            node[i].rstrength[j] = STRENGTHINIT;
        //Ŀ�꼯
        for( j = 0; j < OBJECTNUM; j++) {
            node[i].SdetectO[j][0] = NONEOID;
            node[i].SdetectO[j][1] = NONEID;
            node[i].StrackO[j][0] = NONEOID;
            node[i].StrackO[j][1] = NONEID;
        }
    }
    printf("����ͷ�ڵ��ʼ���ɹ���\n");
    return 0;
}

/************************************************************************
*   ����ͷ�ڵ��ܿ�
*   ���룺�ڵ㣬Ŀ����Ϣ,��Ϣ
*   �����ڵ��߳�
****/
void CameraControl( Node *node, int nodemsg[][NODENUM], TaskInfo taskmsg[][OBJECTNUM], Object *obj )
{
    int i, ret;
    CameraThreadArg nodemsgarg[NODENUM], nodetaskarg[NODENUM], objectdectart[NODENUM];
    pthread_t nodemsgt[NODENUM], nodetaskt[NODENUM], objectdectt[NODENUM];
    pthread_attr_t attr;

    //init
    pthread_attr_init(&attr);
    // explicitly specify as joinable PTHREAD_CREATE_JOINABLE or detached  PTHREAD_CREATE_DETACHED
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    /* �̲߳����ӿ� */
    for( i = 0; i < NODENUM; i++ )
    {
        nodemsgarg[i].node = &node[i];
        nodemsgarg[i].nodemsg = nodemsg;
        nodetaskarg[i].node = &node[i];
        nodetaskarg[i].nodemsg = nodemsg;
        nodetaskarg[i].taskmsg = taskmsg;
        objectdectart[i].node = &node[i];
        objectdectart[i].nodemsg = nodemsg;
        objectdectart[i].taskmsg = taskmsg;
        objectdectart[i].object = obj;
    }

    /* ���������ڵ��߳� */
    for( i = 0; i < NODENUM; i++ )
    {
        //NodeMsgThread�߳�
        ret = pthread_create(&nodemsgt[i], &attr, (void *)NodeMsgThread, &nodemsgarg[i]);
        if(ret!=0) {
            printf("Create NodeMsgThread %d error!\n", i);
            exit(1);
        }
        printf("�ڵ� %d NodeMsgThread�̴߳�����\n", node[i].nodeid );
        //NodeTaskThread�߳�
        ret = pthread_create(&nodetaskt[i], &attr, (void *)NodeTaskThread, &nodetaskarg[i]);
        if(ret!=0) {
            printf("Create NodeTaskThread %d error!\n", i);
            exit(1);
        }
        printf("�ڵ� %d NodeTaskThread�̴߳�����\n", node[i].nodeid );
        //ObjcetDetectThread�߳�
        ret = pthread_create(&objectdectt[i], &attr, (void *)ObjcetDetectThread, &objectdectart[i]);
        if(ret!=0) {
            printf("Create ObjcetDetectThread %d error!\n", i);
            exit(1);
        }
        printf("�ڵ� %d ObjcetDetectThread�̴߳�����\n", node[i].nodeid );
    }
    //release thread attribute
    pthread_attr_destroy(&attr);
}

/*************************************************************************************
*   �ڵ��߳�1���ڵ�䷴����Ϣ����
*   ���룺�ڵ���Ϣ��������Ϣ
*
*****/
static void NodeMsgThread( void *arg )
{
    //�ӿڲ���
    CameraThreadArg *psarg;
    Node *node;
    int **nmsg;

    //��������
    psarg = (CameraThreadArg *)arg;
    node = psarg->node;
    nmsg = psarg->nodemsg;

    //������Ϣ����
    check_nodemsg( node, nmsg );
}

/******������Ϣ����******/
static void check_nodemsg( Node *node, int nmsg[][NODENUM] )
{
    int i,n,nodeid;

    nodeid = node->nodeid;
    n = nodeid - NODEID;//ȡ�ڵ�Ŷ�Ӧ����Ϣλ��

    while(1)
    {
        /* ɨ�跴����Ϣ�� */
        for(i = 0; i < NODENUM; i++)
        {
            if(i == n)
                continue;
            /* �ж���Ϣ���� */
            if(nmsg[n][i] == SUCCESSA) {
                node->rstrength[i] += STRENGTHINCRE; //�޸�ǿ��
                printf("�ڵ�%d �� �ڵ�%d ��ϵ��ǿ��%f\n",(NODEID+i), nodeid, node->rstrength[i]);
            }
            else if(nmsg[n][i] == FAILA) {
                node->rstrength[i] -= (STRENGTHINCRE * STRENGTHRATIO); //�޸�ǿ��
                printf("�ڵ�%d �� �ڵ�%d ��ϵ������%f\n",(NODEID+i), nodeid, node->rstrength[i]);
            }
            nmsg[n][i] = NONEA;    //��Ϣ��ʼ��
        }
        //usleep(SLEEPT);
        //sleep(1);
    }

}

/*************************************************************************************************
*   �ڵ��߳�2:�ڵ�������Ϣ����
*   ���룺�ڵ���Ϣ,������Ϣ,������Ϣ
*
*******/
static void NodeTaskThread( void *arg )
{
    //�ӿڲ���
    CameraThreadArg *psarg;
    Node *node;
    TaskInfo **tmsg;
    int **nmsg;

    //��������
    psarg = (CameraThreadArg *)arg;
    node = psarg->node;
    nmsg = psarg->nodemsg;
    tmsg = psarg->taskmsg;

    //������Ϣ����
    check_taskmsg( node, nmsg, tmsg );
}

/******������Ϣ����******/
static void check_taskmsg( Node *node, int nmsg[][NODENUM], TaskInfo tmsg[][OBJECTNUM] )
{
    int i,n,nodeid,oid,m;

    nodeid = node->nodeid;
    n = nodeid - NODEID;    //ȡ�ڵ�Ŷ�Ӧ����Ϣλ��
    oid = i + OBJECTID; //ȡĿ��λ�ö�Ӧ��Ŀ����
    while(1)
    {
        //sleep(1);
        /* ɨ��������Ϣ�� */
        for(i = 0; i < OBJECTNUM; i++)
        {
            if( tmsg[n][i].tasktype == TRACKTASK )
            {
                //Ŀ���Ѿ��ڸ��ټ���
                if( tmsg[n][i].objectid == node->StrackO[i][0] )
                {
                    m = node->StrackO[i][1] - NODEID;   //ȡ֮ǰԴ�ڵ�Ŷ�Ӧ����Ϣλ��
                    nmsg[m][n] = FAILA;  //n�ڵ㷢����Ϣ��m�ڵ�
                    count_communication(1);
                }
                //���¸��ټ�
                node->StrackO[i][0] = tmsg[n][i].objectid;
                node->StrackO[i][1] = tmsg[n][i].nodeid;
                printf("�ڵ� %d �������񼯸��£�����Ŀ�꣺%d �����ڵ� %d\n",nodeid, node->StrackO[i][0], node->StrackO[i][1]);
            }
            //��Ϣ����ʼ��
            tmsg[n][i].nodeid = NONEID;
            tmsg[n][i].tasktype = NONEA;
        }
        //usleep(SLEEPT);
        //sleep(1);
    }

}

/*************************************************************************************************
*   �ڵ��߳�3��Ŀ����
*   ���룺�ڵ���Ϣ��Ŀ����Ϣ
*   �����������Ϣ��������Ϣ
*******/
static void ObjcetDetectThread( void *arg )
{
    //�ӿڲ���
    CameraThreadArg *psarg;
    Node *node;
    Object *obj;
    TaskInfo **tmsg;
    int **nmsg;

    //��������
    psarg = (CameraThreadArg *)arg;
    node = psarg->node;
    obj = psarg->object;
    nmsg = psarg->nodemsg;
    tmsg = psarg->taskmsg;

    //Ŀ����
    ObjcetDetect( node, nmsg, tmsg, obj );
}

/********�ڵ�Ŀ����*******/
static void ObjcetDetect( Node *node, int nmsg[][NODENUM], TaskInfo tmsg[][OBJECTNUM], Object *obj )
{
    int i,n,nodeid,m;
    int tag[OBJECTNUM];

    nodeid = node->nodeid;
    n = nodeid - NODEID;    //ȡ�ڵ�Ŷ�Ӧ����Ϣλ��

    for(i = 0; i < OBJECTNUM; i++)
        tag[i] = 0;

    while(1)
    {
        //sleep(1);
        for( i = 0; i < OBJECTNUM; i++ )
        {
            if( obj[i].coo.x == node->coo.x && obj[i].coo.y == node->coo.y ) //Ŀ���ڽڵ�FOV��
            {
                if(!tag[i]) {
                    printf("Ŀ�� %d ���� �ڵ� %d FOV\n", obj[i].objectid, nodeid );
                    tag[i] = 1;
                }

                if( obj[i].objectid == node->SdetectO[i][0] ) //Ŀ���Ѿ���⵽
                    continue;
                else
                {
                    if( obj[i].objectid == node->StrackO[i][0] ) //Ŀ���ڸ��ټ���
                    {
                        /* Ŀ������⼯ */
                        node->SdetectO[i][0] = obj[i].objectid;
                        node->SdetectO[i][1] = nodeid;
                        /* ��Դ�ڵ㷴����Ϣ */
                        m = node->StrackO[i][1] - NODEID;
                        nmsg[m][n] = SUCCESSA;
                        /* ����������Ϣ */
                        post_task( tmsg, node, i);
                    }
                    else    //�״μ�⵽Ŀ��
                    {
                        /* Ŀ������⼯ */
                        node->SdetectO[i][0] = obj[i].objectid;
                        node->SdetectO[i][1] = nodeid;
                        /* ����������Ϣ */
                        post_task( tmsg, node, i);
                    }
                }
            }
            else
            {
                tag[i] = 0;
                node->SdetectO[i][0] = NONEOID; //��ʼ��
                node->SdetectO[i][1] = NONEID;
            }
        }
        //usleep(SLEEPT);
        //sleep(1);
    }
}

/*****����������Ϣ*****/
static void post_task( TaskInfo tmsg[][OBJECTNUM], Node *node, int i)
{
    int j, n, nodeid, percent;

    nodeid = node->nodeid;
    n = nodeid - NODEID;    //ȡ�ڵ�Ŷ�Ӧ����Ϣλ��

    for( j = 0; j < NODENUM; j++ )
    {
        if( j == n)
            continue;
        if( node->rstrength[j] >= TVALUE ) //������ֵ
        {
            /* 100%����������Ϣ */
            tmsg[j][i].nodeid = nodeid;
            tmsg[j][i].tasktype = TRACKTASK;
            tmsg[j][i].objectid = i + OBJECTID;
            count_communication(1);
            printf("�ڵ� %d �� �ڵ� %d ���� Ŀ�� %d ��������\n", nodeid, j+NODEID, tmsg[j][i].objectid );
        }
        else {
            /* �����ʷ���������Ϣ */
            srand((unsigned)time(NULL));
            percent = 1 / PVALUE;
            if( 1 == rand() % percent)
            {
                tmsg[j][i].nodeid = nodeid;
                tmsg[j][i].tasktype = TRACKTASK;
                tmsg[j][i].objectid = i + OBJECTID;
                count_communication(1);
                printf("�ڵ� %d �� �ڵ� %d ���� Ŀ�� %d ��������\n", nodeid, j+NODEID, tmsg[j][i].objectid );
            }
        }
    }
}

/*****ͨ�Ŵ���ͳ��*****/
int count_communication(int flag)
{
    static int n = 0;
    if(flag)
        ++n;
    return n;
}

