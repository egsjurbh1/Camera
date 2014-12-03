/** \brief 摄像头节点
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

static void NodeMsgThread(void *arg);       //节点反馈消息处理线程
static void NodeTaskThread(void *arg);      //节点任务消息处理线程
static void ObjcetDetectThread(void *arg);  //节点目标检测线程

static void check_nodemsg(Node *node, SystemPara *sys);
static void check_taskmsg( Node *node, SystemPara *sys);
static void ObjcetDetect( Node *node, Object *obj, SystemPara *sys );
static void post_task( Node *node, int i, SystemPara *sys );
static int isinfov( Node *node, Object *obj);
static int countUtility( Node *node, Object *obj, float *obju );
float count_communication(int flag, int type, float *ic, float taskcost, float feedbackcost);

/**< 线程通信全局量 */
/*NodeMsg[i][j]存储节点j发给节点i的消息*/
static volatile int NodeMsg[MAXNODE][MAXNODE];
/*TaskMsg[i][j]存储节点i收到目标j的任务消息*/
static volatile TaskInfo TaskMsg[MAXNODE][MAXOBJECT];

/***********************************************************
*   摄像头节点初始化函数
*   input:消息参数，节点
*****/
int CameraInit(Node *node, Cross *cross, SystemPara *sys)
{
    int i,j;
    int nodenum,objectnum,commu_mode,fovhalfwidth,fovlength,firstnodeid;
    float strengthinit;

    /* 取系统参数 */
    nodenum = sys->Node.nodenum;
    objectnum = sys->Object.objectnum;
    commu_mode = sys->Node.commu_mode;
    fovhalfwidth = sys->Node.fovhalfwidth;
    fovlength = sys->Node.fovlength;
    firstnodeid = sys->Node.nodeid_init;
    strengthinit = sys->Node.strengthinit;
    /* NodeMsg消息初始化 */
    for( i = 0; i < nodenum; i++)
        for( j = 0; j < nodenum; j++)
            NodeMsg[i][j] = NONEA;
    /* TaskMsg消息初始化 */
    for( i = 0; i < objectnum; i++)
        for( j = 0; j < nodenum; j++) {
            TaskMsg[i][j].tasktype = NONEA;
            TaskMsg[i][j].nodeid = NONEID;
        }

    /* 摄像头节点参数初始化*/
    for( i = 0; i < nodenum; i++, firstnodeid++) {
        node[i].nodeid = firstnodeid;   //节点ID,增量为1
        node[i].commu_mode = commu_mode;
        node[i].load = 0;
        node[i].utility = 0;
        node[i].fovhalfwidth = fovhalfwidth;
        node[i].fovlength = fovlength;

        //邻接表关系强度
        node[i].rstrength = (float *)malloc(nodenum * sizeof(float));
        for( j = 0; j < nodenum; j++)
            node[i].rstrength[j] = strengthinit;
        //目标集
        node[i].SdetectO = (TaskSet *)malloc( objectnum * sizeof(TaskSet));
        node[i].StrackO = (TaskSet *)malloc( objectnum * sizeof(TaskSet));
        for( j = 0; j < objectnum; j++) {
            node[i].SdetectO[j].objectid = NONEID;
            node[i].SdetectO[j].postnodeid = NONEID;
            node[i].StrackO[j].objectid = NONEID;
            node[i].StrackO[j].postnodeid = NONEID;
        }
    }

    /* 摄像头节点位置初始化*/
    //节点C1(路口2 西)
    node[0].coo.x = cross[1].coo.x - cross[1].halfwidth;
    node[0].coo.y = cross[1].coo.y;
    node[0].direction = WEST;
    //节点C2(路口1 东)
    node[1].coo.x = cross[0].coo.x + cross[0].halfwidth;
    node[1].coo.y = cross[0].coo.y;
    node[1].direction = EAST;
    //节点C3(路口1 北)
    node[2].coo.x = cross[0].coo.x;
    node[2].coo.y = cross[0].coo.y + cross[0].halfwidth;
    node[2].direction = NORTH;
    //节点C4(路口1 西)
    node[3].coo.x = cross[0].coo.x - cross[0].halfwidth;
    node[3].coo.y = cross[0].coo.y;
    node[3].direction = WEST;
    //节点C5(路口1 南)
    node[4].coo.x = cross[0].coo.x;
    node[4].coo.y = cross[0].coo.y - cross[0].halfwidth;
    node[4].direction = SOUTH;
    //节点C6(路口2 北)
    node[5].coo.x = cross[1].coo.x;
    node[5].coo.y = cross[1].coo.y + cross[1].halfwidth;
    node[5].direction = NORTH;
    //节点C7(路口2 东)
    node[6].coo.x = cross[1].coo.x + cross[1].halfwidth;
    node[6].coo.y = cross[1].coo.y;
    node[6].direction = EAST;
    //节点C8(路口2 南)
    node[7].coo.x = cross[1].coo.x;
    node[7].coo.y = cross[1].coo.y - cross[1].halfwidth;
    node[7].direction = SOUTH;
    printf("Camera Node Initialize Success.\n");
    return 0;
}

/************************************************************************
*   摄像头节点总控
*   输入：节点，目标信息,消息
*   创建节点线程
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

    /* 线程参数接口 */
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

    /* 创建各个节点线程 */
    for( i = 0; i < nodenum; i++ )
    {
        //NodeMsgThread线程
        ret = pthread_create(&nodemsgt[i], &attr, (void *)NodeMsgThread, &nodemsgarg[i]);
        if(ret!=0) {
            printf("Create NodeMsgThread %d error!\n", i);
            exit(1);
        }
        sleep(1);
        //NodeTaskThread线程
        ret = pthread_create(&nodetaskt[i], &attr, (void *)NodeTaskThread, &nodetaskarg[i]);
        if(ret!=0) {
            printf("Create NodeTaskThread %d error!\n", i);
            exit(1);
        }
        sleep(1);
        //ObjcetDetectThread线程
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
*   节点线程1：节点间反馈消息处理
*
*****/
static void NodeMsgThread( void *arg )
{
    //接口参数
    CameraThreadArg *psarg;
    Node *node;
    SystemPara *sys;

    //参数引用
    psarg = (CameraThreadArg *)arg;
    node = psarg->node;
    sys = psarg->sys;

    //反馈消息处理
    check_nodemsg( node, sys );
}

/******反馈消息处理******/
static void check_nodemsg(Node *node, SystemPara *sys)
{
    int i,n,j,nodeid;
    int nodeid_init,nodenum;
    float strengthincre,strengthratio,maxsvalue,minsvalue;
    /* 取系统参数 */
    nodeid_init = sys->Node.nodeid_init;
    nodenum = sys->Node.nodenum;
    strengthratio = sys->Node.strengthratio;
    strengthincre = sys->Node.strengthincre;
    maxsvalue = sys->Node.maxvalue;
    minsvalue = sys->Node.minvalue;

    nodeid = node->nodeid;
    n = nodeid - nodeid_init;//取节点号对应的消息位置

    while(1)
    {
        /* 扫描反馈消息表 */
        for(i = 0; i < nodenum; i++)
        {
            if(i == n)
                continue;

            /* 判断消息类型 */
            if(NodeMsg[n][i] == SUCCESSA) {
                node->rstrength[i] += strengthincre; //修改强度
                //printf("节点%d 对 节点%d 关系增强，%.2f\n",(nodeid_init+i), nodeid, node->rstrength[i]);
            }
            else if(NodeMsg[n][i] == FAILA) {
                node->rstrength[i] -= (strengthincre * strengthratio); //修改强度
                //printf("节点%d 对 节点%d 关系减弱，%.2f\n",(nodeid_init+i), nodeid, node->rstrength[i]);
            }

            /* strength值范围控制 */
            if(node->rstrength[i] >= maxsvalue)
                node->rstrength[i] = maxsvalue;
            else if(node->rstrength[i] <= minsvalue)
                node->rstrength[i] = minsvalue;
            NodeMsg[n][i] = NONEA;    //消息初始化
        }
        sleep(1);
    }

}

/*************************************************************************************************
*   节点线程2:节点任务消息处理
*
*******/
static void NodeTaskThread( void *arg )
{
    //接口参数
    CameraThreadArg *psarg;
    Node *node;
    SystemPara *sys;

    //参数引用
    psarg = (CameraThreadArg *)arg;
    node = psarg->node;
    sys = psarg->sys;

    //任务消息处理
    check_taskmsg(node, sys);
}

/******任务消息处理******/
static void check_taskmsg( Node *node, SystemPara *sys )
{
    int i,n,nodeid,m;
    float it,feedbackcost;
    int nodeid_init,objectnum;

    /* 取系统参数 */
    nodeid_init = sys->Node.nodeid_init;
    objectnum = sys->Object.objectnum;
    feedbackcost = sys->Node.feedbackcost;

    nodeid = node->nodeid;
    n = nodeid - nodeid_init;    //取节点号对应的消息位置
    while(1)
    {
        /* 扫描任务消息表 */
        for(i = 0; i < objectnum; i++)
        {
            if( TaskMsg[n][i].tasktype == TRACKTASK )
            {
                //目标已经在跟踪集中
                if( TaskMsg[n][i].objectid == node->StrackO[i].objectid )
                {
                    m = node->StrackO[i].postnodeid - nodeid_init;   //取之前源节点号对应的消息位置
                    NodeMsg[m][n] = FAILA;  //n节点发送消息给m节点
                    count_communication(1,FEEDBACKCOMMU,&it,0,feedbackcost);
                }
                //更新跟踪集
                node->StrackO[i].objectid = TaskMsg[n][i].objectid;
                node->StrackO[i].postnodeid = TaskMsg[n][i].nodeid;
                //printf("节点 %d 跟踪任务集更新，跟踪目标：%d 发布节点 %d\n",nodeid, node->StrackO[i].objectid, node->StrackO[i].postnodeid);
            }
            //消息区初始化
            TaskMsg[n][i].nodeid = NONEID;
            TaskMsg[n][i].tasktype = NONEA;
        }
        sleep(1);
    }

}

/*************************************************************************************************
*   节点线程3：目标检测
*******/
static void ObjcetDetectThread( void *arg )
{
    //接口参数
    CameraThreadArg *psarg;
    Node *node;
    Object *obj;
    SystemPara *sys;

    //参数引用
    psarg = (CameraThreadArg *)arg;
    node = psarg->node;
    obj = psarg->object;
    sys = psarg->sys;

    //目标检测
    ObjcetDetect( node, obj, sys );
}

/********节点目标检测*******/
static void ObjcetDetect( Node *node, Object *obj, SystemPara *sys )
{
    int i,n,nodeid,m;
    int nodeid_init,objectnum,maxnum;
    float maxload;
    /* 取系统参数 */
    nodeid_init = sys->Node.nodeid_init;
    objectnum = sys->Object.objectnum;
    maxload = sys->Node.maxload;
    maxnum = sys->Node.maxnum;

    int tag[objectnum];
    float obju[objectnum];

    nodeid = node->nodeid;
    n = nodeid - nodeid_init;    //取节点号对应的消息位置

    for(i = 0; i < objectnum; i++)
        tag[i] = 0;

    while(1)
    {
        node->utility = 0; //检测目标前，效用值初始化
        for( i = 0; i < objectnum; i++ )
        {
            if( isinfov( node, &obj[i]) ) //目标在节点FOV内
            {
                //节点超负载，放弃目标跟踪
                if( node->load >= maxload) {
                    node->load = maxload;
                    continue;
                }

                if(!tag[i]) {
                    node->load = (node->load * maxnum + 1) / maxnum;    //更新节点负载
                    printf("Object:%d in Node:%d FOV\n", obj[i].objectid, nodeid );
                    tag[i] = 1;
                }
                countUtility( node, &obj[i], &obju[i]); //计算效用

                if( obj[i].objectid == node->SdetectO[i].objectid ) //目标已加入到目标集
                    continue;
                else    //新任务
                {
                    if( obj[i].objectid == node->StrackO[i].objectid ) //目标在跟踪集中
                    {
                        /* 目标加入检测集 */
                        node->SdetectO[i].objectid = obj[i].objectid;
                        node->SdetectO[i].postnodeid = nodeid;
                        /* 向源节点反馈消息 */
                        m = node->StrackO[i].postnodeid - nodeid_init;
                        NodeMsg[m][n] = SUCCESSA;
                        /* 发布任务消息 */
                        post_task( node, i, sys );
                    }
                    else    //首次检测到目标
                    {
                        /* 目标加入检测集 */
                        node->SdetectO[i].objectid = obj[i].objectid;
                        node->SdetectO[i].postnodeid = nodeid;
                        /* 发布任务消息 */
                        post_task( node, i, sys );
                    }
                }
            }
            else
            {
                if(tag[i])
                    node->load = (node->load * maxnum - 1) / maxnum;    //更新节点负载
                tag[i] = 0;
                node->SdetectO[i].objectid = NONEID; //初始化
                node->SdetectO[i].postnodeid = NONEID;
                obj[i].distance = 0;
                obju[i] = 0;
            }
            node->utility += obju[i];   //节点总效用更新
        }
        sleep(1);
    }
}

/*****发布任务消息*****/
static void post_task( Node *node, int i, SystemPara *sys )
{
    int j, n, nodeid, percent;
    int mode, nodenum, nodeid_init, objectid_init;
    float it, tvalue, pvalue, taskcost, feedbackcost;
    /* 取系统参数 */
    nodeid_init = sys->Node.nodeid_init;
    objectid_init = sys->Object.objectid_init;
    tvalue = sys->Node.tvalue;
    pvalue = sys->Node.pvalue;
    nodenum = sys->Node.nodenum;
    taskcost = sys->Node.taskcost;
    feedbackcost = sys->Node.feedbackcost;

    nodeid = node->nodeid;
    mode = node->commu_mode;
    n = nodeid - nodeid_init;    //取节点号对应的消息位置

    for( j = 0; j < nodenum; j++ )
    {
        if( j == n)
            continue;
        switch(mode) {
        case MULTICAST:
            if( node->rstrength[j] >= tvalue ) //大于阈值
            {
                /* 100%发送任务消息 */
                TaskMsg[j][i].nodeid = nodeid;
                TaskMsg[j][i].tasktype = TRACKTASK;
                TaskMsg[j][i].objectid = i + objectid_init;
                count_communication(1,TASKCOMMU,&it,taskcost,feedbackcost);
                //printf("节点 %d 向 节点 %d 发布 目标 %d 跟踪任务\n", nodeid, j+NODEID, TaskMsg[j][i].objectid );
            }
            else {
                /* 按概率发送任务消息 */
                srand((unsigned)time(NULL));
                percent = 1 / pvalue;
                if( 1 == rand() % percent)
                {
                    TaskMsg[j][i].nodeid = nodeid;
                    TaskMsg[j][i].tasktype = TRACKTASK;
                    TaskMsg[j][i].objectid = i + objectid_init;
                    count_communication(1,TASKCOMMU,&it,taskcost,feedbackcost);
                    //printf("节点 %d 向 节点 %d 发布 目标 %d 跟踪任务\n", nodeid, j+NODEID, TaskMsg[j][i].objectid );
                }
            }
            break;

        case BROADCAST:
            /* 100%发送任务消息 */
            TaskMsg[j][i].nodeid = nodeid;
            TaskMsg[j][i].tasktype = TRACKTASK;
            TaskMsg[j][i].objectid = i + objectid_init;
            count_communication(1,TASKCOMMU,&it,taskcost,feedbackcost);
            //printf("节点 %d 向 节点 %d 发布 目标 %d 跟踪任务\n", nodeid, j+NODEID, TaskMsg[j][i].objectid );
            break;

        default:
            break;
        }
    }
}
/*****判断目标是否在FOV内*****/
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

/*****通信次数统计*****/
/**< 输入：flag 0 输出， 1 计数
*          type 通信类型 TASKCOMMU, FEEDBACKCOMMU
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
        in = 0; //瞬时统计量归零
    }
    return n;
}

/*****目标瞬时效用计算*****/
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

    //计算目标节点距离
    obj->distance = sqrt((ox - nx)*(ox - nx) + (oy - ny)*(oy - ny));
    //计算当前效用
    *obju = (1 - load)*(1 - obj->distance / D);

    return 0;
}

