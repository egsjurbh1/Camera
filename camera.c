/** \brief Camera Node Control
 *
 * \author chinglee
 * \update 150102
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
static void setNode(SystemPara *sys, Node *node, MapCoo coo, int direction);
static void setNodeArray(int array_n, SystemPara *sys, Node *node, Cross *cross);

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
    int i,j,ret;
    int systemmode, nodenum, objectnum;

    /* 取系统参数 */
    systemmode = sys->system_mode;
    nodenum = sys->Node.nodenum;
    objectnum = sys->Object.objectnum;

    switch(systemmode)
    {
        ///跟踪模式
        case TRACK_MODE:
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
            setNodeArray(1, sys, node, cross);
            ret = 1;
            break;
        ///任务分配模式
        case TASK_MODE:
            setNodeArray(2, sys, node, cross);
            ret = 1;
            break;
        default:
            ret = 0;
            break;
    }

    if(ret)
        printf("Camera Node Initialize Success.\n");
    else
        printf("Camera Node Initialize Failed.\n");

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
    int systemmode = sys->system_mode;

    //非跟踪模式
    if( systemmode != TRACK_MODE)
        return;

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
        printf("Node:%d Created.\n", node[i].nodeid );
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
    int i,n,nodeid;
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
        if(obj->coo.y >= node->coo.y && obj->coo.y <= (node->coo.y + node->fovlength) &&
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

/**< 节点配置 */
static void setNode(SystemPara *sys, Node *node, MapCoo coo, int direction)
{
    int i,nodenum,objectnum,commu_mode,fovhalfwidth,fovlength,firstnodeid,tasknum;
    float strengthinit;
    static int n = 0;
    nodenum = sys->Node.nodenum;
    objectnum = sys->Object.objectnum;
    commu_mode = sys->Node.commu_mode;
    fovhalfwidth = sys->Node.fovhalfwidth;
    fovlength = sys->Node.fovlength;
    firstnodeid = sys->Node.nodeid_init;
    strengthinit = sys->Node.strengthinit;
    tasknum = sys->Task.tasknum;

    if(n > nodenum){
        printf("!WARNING:Object Number Extra.\n");
        return;
    }

    node[n].nodeid = firstnodeid + n;   //节点ID,增量为1
    node[n].commu_mode = commu_mode;
    node[n].load = 0;
    node[n].tasknum = 0;
    node[n].utility = 0;
    node[n].fovhalfwidth = fovhalfwidth;
    node[n].fovlength = fovlength;

    //邻接表关系强度
    node[n].rstrength = (float *)malloc(nodenum * sizeof(float));
    for( i = 0; i < nodenum; i++)
        node[n].rstrength[i] = strengthinit;
    //目标集
    node[n].SdetectO = (TaskSet *)malloc( objectnum * sizeof(TaskSet));
    node[n].StrackO = (TaskSet *)malloc( objectnum * sizeof(TaskSet));
    for( i = 0; i < objectnum; i++) {
        node[n].SdetectO[i].objectid = NONEID;
        node[n].SdetectO[i].postnodeid = NONEID;
        node[n].StrackO[i].objectid = NONEID;
        node[n].StrackO[i].postnodeid = NONEID;
    }
    //任务集
    node[n].taskid = (int *)malloc( tasknum * sizeof(int));
    node[n].taskid[0] = 0;
    //位置
    node[n].coo.x = coo.x;
    node[n].coo.y = coo.y;
    node[n].direction = direction;

    ++n;
}

/**< 节点组配置 */
static void setNodeArray(int array_n, SystemPara *sys, Node *node, Cross *cross)
{
    int i;
    MapCoo c1_north, c1_south, c1_east, c1_west, c2_north, c2_south, c2_east, c2_west;

    /* 路口方位坐标 */
    c1_north.x = c1_south.x = cross[0].coo.x;
    c1_north.y = cross[0].coo.y + cross[0].halfwidth;
    c1_south.y = cross[0].coo.y - cross[0].halfwidth;
    c1_east.x = cross[0].coo.x + cross[0].halfwidth;
    c1_west.x = cross[0].coo.x - cross[0].halfwidth;
    c1_east.y = c1_west.y = cross[0].coo.y;

    c2_north.x = c2_south.x = cross[1].coo.x;
    c2_north.y = cross[1].coo.y + cross[1].halfwidth;
    c2_south.y = cross[1].coo.y - cross[1].halfwidth;
    c2_east.x = cross[1].coo.x + cross[1].halfwidth;
    c2_west.x = cross[1].coo.x - cross[1].halfwidth;
    c2_east.y = c2_west.y = cross[1].coo.y;

    for(i = 0; i < array_n; ++i)
    {
        //一组配置2个路口的四个方位共八个点
        setNode( sys, node, c1_north, NORTH);
        setNode( sys, node, c1_south, SOUTH);
        setNode( sys, node, c1_west, WEST);
        setNode( sys, node, c1_east, EAST);
        setNode( sys, node, c2_north, NORTH);
        setNode( sys, node, c2_south, SOUTH);
        setNode( sys, node, c2_west, WEST);
        setNode( sys, node, c2_east, EAST);
    }
}


