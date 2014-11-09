/** \brief 摄像头节点
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

static void NodeMsgThread(void *arg);  //节点反馈消息处理线程
static void NodeTaskThread(void *arg); //节点任务消息处理线程
static void ObjcetDetectThread(void *arg); //节点目标检测线程

static void check_nodemsg( Node *node, int nmsg[][NODENUM] );
static void check_taskmsg( Node *node, int nmsg[][NODENUM], TaskInfo tmsg[][OBJECTNUM] );
static void ObjcetDetect( Node *node, int nmsg[][NODENUM], TaskInfo tmsg[][OBJECTNUM], Object *obj );
static void post_task( TaskInfo tmsg[][OBJECTNUM], Node *node, int i);
int count_communication(int flag);

/***********************************************************
*   摄像头节点初始化函数
*   input:消息参数，节点
*****/
int CameraInit( Node *node, int nodemsg[][NODENUM], TaskInfo taskmsg[][OBJECTNUM])
{
    int i,j;
    int firstnodeid = NODEID;
    const int nodecoo[NODENUM][2] = {{3,1},{2,1},{1,2},{0,1},{1,0}};//节点坐标

    /* NodeMsg消息初始化 */
    for( i = 0; i < NODENUM; i++)
        for( j = 0; j < NODENUM; j++)
            nodemsg[i][j] = NONEA;
    /* TaskMsg消息初始化 */
    for( i = 0; i < OBJECTNUM; i++)
        for( j = 0; j < NODENUM; j++) {
            taskmsg[i][j].tasktype = NONEA;
            taskmsg[i][j].nodeid = NONEID;
        }

    /* 摄像头节点参数初始化*/
    for( i = 0; i < NODENUM; i++, firstnodeid++) {
        //节点ID,增量为1
        node[i].nodeid = firstnodeid;
        //节点坐标初始化
        node[i].coo.x = nodecoo[i][0];
        node[i].coo.y = nodecoo[i][1];
        //邻接表关系强度
        for( j = 0; j < NODENUM; j++)
            node[i].rstrength[j] = STRENGTHINIT;
        //目标集
        for( j = 0; j < OBJECTNUM; j++) {
            node[i].SdetectO[j][0] = NONEOID;
            node[i].SdetectO[j][1] = NONEID;
            node[i].StrackO[j][0] = NONEOID;
            node[i].StrackO[j][1] = NONEID;
        }
    }
    printf("摄像头节点初始化成功。\n");
    return 0;
}

/************************************************************************
*   摄像头节点总控
*   输入：节点，目标信息,消息
*   创建节点线程
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

    /* 线程参数接口 */
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

    /* 创建各个节点线程 */
    for( i = 0; i < NODENUM; i++ )
    {
        //NodeMsgThread线程
        ret = pthread_create(&nodemsgt[i], &attr, (void *)NodeMsgThread, &nodemsgarg[i]);
        if(ret!=0) {
            printf("Create NodeMsgThread %d error!\n", i);
            exit(1);
        }
        printf("节点 %d NodeMsgThread线程创建。\n", node[i].nodeid );
        //NodeTaskThread线程
        ret = pthread_create(&nodetaskt[i], &attr, (void *)NodeTaskThread, &nodetaskarg[i]);
        if(ret!=0) {
            printf("Create NodeTaskThread %d error!\n", i);
            exit(1);
        }
        printf("节点 %d NodeTaskThread线程创建。\n", node[i].nodeid );
        //ObjcetDetectThread线程
        ret = pthread_create(&objectdectt[i], &attr, (void *)ObjcetDetectThread, &objectdectart[i]);
        if(ret!=0) {
            printf("Create ObjcetDetectThread %d error!\n", i);
            exit(1);
        }
        printf("节点 %d ObjcetDetectThread线程创建。\n", node[i].nodeid );
    }
    //release thread attribute
    pthread_attr_destroy(&attr);
}

/*************************************************************************************
*   节点线程1：节点间反馈消息处理
*   输入：节点信息，反馈消息
*
*****/
static void NodeMsgThread( void *arg )
{
    //接口参数
    CameraThreadArg *psarg;
    Node *node;
    int **nmsg;

    //参数引用
    psarg = (CameraThreadArg *)arg;
    node = psarg->node;
    nmsg = psarg->nodemsg;

    //反馈消息处理
    check_nodemsg( node, nmsg );
}

/******反馈消息处理******/
static void check_nodemsg( Node *node, int nmsg[][NODENUM] )
{
    int i,n,nodeid;

    nodeid = node->nodeid;
    n = nodeid - NODEID;//取节点号对应的消息位置

    while(1)
    {
        /* 扫描反馈消息表 */
        for(i = 0; i < NODENUM; i++)
        {
            if(i == n)
                continue;
            /* 判断消息类型 */
            if(nmsg[n][i] == SUCCESSA) {
                node->rstrength[i] += STRENGTHINCRE; //修改强度
                printf("节点%d 对 节点%d 关系增强，%f\n",(NODEID+i), nodeid, node->rstrength[i]);
            }
            else if(nmsg[n][i] == FAILA) {
                node->rstrength[i] -= (STRENGTHINCRE * STRENGTHRATIO); //修改强度
                printf("节点%d 对 节点%d 关系减弱，%f\n",(NODEID+i), nodeid, node->rstrength[i]);
            }
            nmsg[n][i] = NONEA;    //消息初始化
        }
        //usleep(SLEEPT);
        //sleep(1);
    }

}

/*************************************************************************************************
*   节点线程2:节点任务消息处理
*   输入：节点信息,任务消息,反馈消息
*
*******/
static void NodeTaskThread( void *arg )
{
    //接口参数
    CameraThreadArg *psarg;
    Node *node;
    TaskInfo **tmsg;
    int **nmsg;

    //参数引用
    psarg = (CameraThreadArg *)arg;
    node = psarg->node;
    nmsg = psarg->nodemsg;
    tmsg = psarg->taskmsg;

    //任务消息处理
    check_taskmsg( node, nmsg, tmsg );
}

/******任务消息处理******/
static void check_taskmsg( Node *node, int nmsg[][NODENUM], TaskInfo tmsg[][OBJECTNUM] )
{
    int i,n,nodeid,oid,m;

    nodeid = node->nodeid;
    n = nodeid - NODEID;    //取节点号对应的消息位置
    oid = i + OBJECTID; //取目标位置对应的目标编号
    while(1)
    {
        //sleep(1);
        /* 扫描任务消息表 */
        for(i = 0; i < OBJECTNUM; i++)
        {
            if( tmsg[n][i].tasktype == TRACKTASK )
            {
                //目标已经在跟踪集中
                if( tmsg[n][i].objectid == node->StrackO[i][0] )
                {
                    m = node->StrackO[i][1] - NODEID;   //取之前源节点号对应的消息位置
                    nmsg[m][n] = FAILA;  //n节点发送消息给m节点
                    count_communication(1);
                }
                //更新跟踪集
                node->StrackO[i][0] = tmsg[n][i].objectid;
                node->StrackO[i][1] = tmsg[n][i].nodeid;
                printf("节点 %d 跟踪任务集更新，跟踪目标：%d 发布节点 %d\n",nodeid, node->StrackO[i][0], node->StrackO[i][1]);
            }
            //消息区初始化
            tmsg[n][i].nodeid = NONEID;
            tmsg[n][i].tasktype = NONEA;
        }
        //usleep(SLEEPT);
        //sleep(1);
    }

}

/*************************************************************************************************
*   节点线程3：目标检测
*   输入：节点信息，目标信息
*   输出：反馈消息，任务消息
*******/
static void ObjcetDetectThread( void *arg )
{
    //接口参数
    CameraThreadArg *psarg;
    Node *node;
    Object *obj;
    TaskInfo **tmsg;
    int **nmsg;

    //参数引用
    psarg = (CameraThreadArg *)arg;
    node = psarg->node;
    obj = psarg->object;
    nmsg = psarg->nodemsg;
    tmsg = psarg->taskmsg;

    //目标检测
    ObjcetDetect( node, nmsg, tmsg, obj );
}

/********节点目标检测*******/
static void ObjcetDetect( Node *node, int nmsg[][NODENUM], TaskInfo tmsg[][OBJECTNUM], Object *obj )
{
    int i,n,nodeid,m;
    int tag[OBJECTNUM];

    nodeid = node->nodeid;
    n = nodeid - NODEID;    //取节点号对应的消息位置

    for(i = 0; i < OBJECTNUM; i++)
        tag[i] = 0;

    while(1)
    {
        //sleep(1);
        for( i = 0; i < OBJECTNUM; i++ )
        {
            if( obj[i].coo.x == node->coo.x && obj[i].coo.y == node->coo.y ) //目标在节点FOV内
            {
                if(!tag[i]) {
                    printf("目标 %d 进入 节点 %d FOV\n", obj[i].objectid, nodeid );
                    tag[i] = 1;
                }

                if( obj[i].objectid == node->SdetectO[i][0] ) //目标已经检测到
                    continue;
                else
                {
                    if( obj[i].objectid == node->StrackO[i][0] ) //目标在跟踪集中
                    {
                        /* 目标加入检测集 */
                        node->SdetectO[i][0] = obj[i].objectid;
                        node->SdetectO[i][1] = nodeid;
                        /* 向源节点反馈消息 */
                        m = node->StrackO[i][1] - NODEID;
                        nmsg[m][n] = SUCCESSA;
                        /* 发布任务消息 */
                        post_task( tmsg, node, i);
                    }
                    else    //首次检测到目标
                    {
                        /* 目标加入检测集 */
                        node->SdetectO[i][0] = obj[i].objectid;
                        node->SdetectO[i][1] = nodeid;
                        /* 发布任务消息 */
                        post_task( tmsg, node, i);
                    }
                }
            }
            else
            {
                tag[i] = 0;
                node->SdetectO[i][0] = NONEOID; //初始化
                node->SdetectO[i][1] = NONEID;
            }
        }
        //usleep(SLEEPT);
        //sleep(1);
    }
}

/*****发布任务消息*****/
static void post_task( TaskInfo tmsg[][OBJECTNUM], Node *node, int i)
{
    int j, n, nodeid, percent;

    nodeid = node->nodeid;
    n = nodeid - NODEID;    //取节点号对应的消息位置

    for( j = 0; j < NODENUM; j++ )
    {
        if( j == n)
            continue;
        if( node->rstrength[j] >= TVALUE ) //大于阈值
        {
            /* 100%发送任务消息 */
            tmsg[j][i].nodeid = nodeid;
            tmsg[j][i].tasktype = TRACKTASK;
            tmsg[j][i].objectid = i + OBJECTID;
            count_communication(1);
            printf("节点 %d 向 节点 %d 发布 目标 %d 跟踪任务\n", nodeid, j+NODEID, tmsg[j][i].objectid );
        }
        else {
            /* 按概率发送任务消息 */
            srand((unsigned)time(NULL));
            percent = 1 / PVALUE;
            if( 1 == rand() % percent)
            {
                tmsg[j][i].nodeid = nodeid;
                tmsg[j][i].tasktype = TRACKTASK;
                tmsg[j][i].objectid = i + OBJECTID;
                count_communication(1);
                printf("节点 %d 向 节点 %d 发布 目标 %d 跟踪任务\n", nodeid, j+NODEID, tmsg[j][i].objectid );
            }
        }
    }
}

/*****通信次数统计*****/
int count_communication(int flag)
{
    static int n = 0;
    if(flag)
        ++n;
    return n;
}

