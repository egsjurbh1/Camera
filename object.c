/*
*   运动目标
*   lq
*   update：141031
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <math.h>
#include "object.h"

static int gValue[MAPX][MAPY]; /* 记录已走过的路 */

static int find_path(int mapblock[][MAPY], int x, int y, MapCoo destco, MapCoo *route);
static int check_pos_valid(int mapblock[][MAPY], int x, int y);
static void MoveMode0Thread( void *arg );
static void MoveMode1Thread( void *arg );
static int find_next(int mapblock[][MAPY], int x, int y, MapCoo *nextco );
static void waitdonothing( int timelength );

/***************************************************************
*目标初始化函数
*****/
int ObjectInit( Object *obj )
{
    int i;
    int objectcoo[OBJECTNUM][2] = {{1,1},{5,1}};  //初始化坐标
    int destcoo[OBJECTNUM][2] = {{5,1},{3,1}};  //目的地坐标
    float speed[2] = {0.5, 0.2};    //目标速度 单位：秒/格

    //初始化参数
    for( i = 0; i < OBJECTNUM; i++)
    {
        obj[i].coo.x = objectcoo[i][0];
        obj[i].coo.y = objectcoo[i][1];
        obj[i].destcoo.x = destcoo[i][0];
        obj[i].destcoo.y = destcoo[i][1];
        obj[i].objectid = OBJECTID + i;
        obj[i].speed = speed[0];
        obj[i].mode = ACTIVEMODE;
        printf("目标%d 初始化成功。\n", obj[i].objectid );
    }
    return 0;
}

/******************************************************************
*   目标运动总控
*   输入：地图,目标信息,控制参数
*   创建目标运动线程
****/
void ObjectMovement( int mapb[][MAPY], Object *obj, int configarg )
{
    int i, ret;
    int mode[OBJECTNUM];
    pthread_t objectthread[OBJECTNUM];
    pthread_attr_t attr;
    MoveArg marg1[OBJECTNUM], marg2[OBJECTNUM];

    //init
    pthread_attr_init(&attr);
    // explicitly specify as joinable PTHREAD_CREATE_JOINABLE or detached  PTHREAD_CREATE_DETACHED
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    /* 线程参数接口 */
    for( i = 0; i < OBJECTNUM; i++ )
    {
        marg1[i].mapblock = mapb;
        marg1[i].obj = &obj[i];
        marg2[i].mapblock = mapb;
        marg2[i].obj = &obj[i];
        mode[i] = obj[i].mode;
    }

    /* 判断控制参数*/
    switch(configarg){
    case STOPACTION: //终止
        exit(1);
        break;
    case EXECUTEACTION: //执行
        break;
    default:
        exit(1);
        break;
    }

    /* 判断运动方式 */
    for( i = 0; i < OBJECTNUM; i++ )
    {
        switch(mode[i]){
        case PASSIVEMODE: //离线路径引导模式
            ret = pthread_create(&objectthread[i], &attr, (void *)MoveMode0Thread, &marg1[i]);    //mode0运动线程
            if(ret!=0) {
                printf("Create MoveMode0Thread error!\n");
                exit(1);
            }
            printf("目标 %d 运动线程（模式:0：路径引导）创建。\n", obj[i].objectid );
            break;
        case ACTIVEMODE: //在线寻路模式
            ret = pthread_create(&objectthread[i], &attr, (void *)MoveMode1Thread, &marg2[i]);    //mode0运动线程
            if(ret!=0) {
                printf("Create MoveMode1Thread error!\n");
                exit(1);
            }
            printf("目标 %d 运动线程（模式1：随机寻路）创建。\n", obj[i].objectid );
            break;
        default:
            break;
        }
    }
    //release thread attribute
    pthread_attr_destroy(&attr);
}

/*************************************
*   目标运动线程1（节点在线主动随机寻路）
*   输入：目标信息,起点,速度(n格/s)
*
*************************************/
static void MoveMode1Thread( void *arg )
{
    MoveArg *psarg;
    int **mapblock;
    Object *obj;

    float speed;
    MapCoo currentco;
    MapCoo nextco;

    //参数引用
    psarg = (MoveArg *)arg;
    mapblock = psarg->mapblock;
    obj = psarg->obj;

    speed = obj->speed;

    printf("目标 %d 开始位置 (%d, %d)\n", obj->objectid, obj->coo.x, obj->coo.y);
    while(1)
    {
        //目标起始坐标
        currentco.x = obj->coo.x;
        currentco.y = obj->coo.y;
        //寻找下一坐标
        if( find_next( mapblock, currentco.x, currentco.y, &nextco ) )
        {
            obj->coo.x = nextco.x;  //移动
            obj->coo.y = nextco.y;
            printf("目标 %d 位置 (%d, %d)\n", obj->objectid, obj->coo.x, obj->coo.y);
            waitdonothing(1/speed); //等待
        }
        //usleep(100);
    }
}

/*****
*   目标移动到下一坐标
*   输入：地图，起点坐标x、y，目标；
*   返回1找到可通行点
*****/
static int find_next(int mapblock[][MAPY], int x, int y, MapCoo *nextco )
{
    int i;
    /* 随机决策下一方位 */
    srand((unsigned)time(NULL));
    i = rand() % PERCENT;

    switch(i){
    case 0:
        if( check_pos_valid(mapblock, x - 1, y) ) {
            nextco->x = x - 1;
            nextco->y = y;
        }
        else
            return 0;
        break;
    case 1:
        if( check_pos_valid(mapblock, x, y - 1) ) {
            nextco->x = x;
            nextco->y = y - 1;
        }
        else
            return 0;
        break;
    case 2:
        if( check_pos_valid(mapblock, x + 1, y) ) {
            nextco->x = x + 1;
            nextco->y = y;
        }
        else
            return 0;
        break;
    case 3:
        if( check_pos_valid(mapblock, x, y + 1) ) {
            nextco->x = x;
            nextco->y = y + 1;
        }
        else
            return 0;
        break;
    default:
        printf("FUCK ERROR!\n");
        return 0;
    }

    return 1;
}

/*******************************************
*   等待函数
*   单位秒
*****/
static void waitdonothing( int timelength )
{
    time_t tb,te;

    time(&tb);
    while(1)
    {
        time(&te);
        if( abs(te - tb) >= timelength )
            break;
    }
}

/******************************
*   目标运动线程0（离线寻路径，按路径引导节点）
*   输入：目标信息,起点,终点,速度(n格/s)
*
*****************************/
static void MoveMode0Thread( void *arg )
{
    MoveArg *psarg;//接口结构
    int **mapblock;
    Object *obj;

    MapCoo destco;
    float speed;
    int i = 0;
    MapCoo startco;
    MapCoo route[20];
    //参数引用
    psarg = (MoveArg *)arg;
    mapblock = psarg->mapblock;
    obj = psarg->obj;
    //目标坐标
    startco.x = obj->coo.x;
    startco.y = obj->coo.y;
    destco.x = obj->destcoo.x;
    destco.y = obj->destcoo.y;
    speed = obj->speed;
    //寻找路径
    find_path( mapblock, startco.x, startco.y, destco, route);
    //目标按路径从起点移动至终点
    printf("目标 %d 开始位置 (%d, %d)\n", obj->objectid, obj->coo.x, obj->coo.y);
    while(1)
    {
        sleep(1);
        if(route[i].x == ENDROUTE)
            break;
        waitdonothing(1/speed); //等待
        ++i;
        obj->coo.x = route[i].x;
        obj->coo.y = route[i].y;
        printf("目标 %d 位置 (%d, %d)\n", obj->objectid, obj->coo.x, obj->coo.y);
    }
}

/******边界检测******/
static int check_pos_valid(int mapblock[][MAPY], int x, int y)
{
    /* 节点是否出边界 */
    if(x < 0 || x > MAPX || y < 0 || y > MAPY)
        return 0;

    /* 当前节点是否存在路 */
    if(mapblock[x][y] == IMPASSABLE)
        return 0;

    /* 当前节点是否已经走过 */
    if(gValue[x][y] == PASSED)
        return 0;

    return 1;
}

/****
*   寻找可通行路径
*   输入：地图，起点坐标x、y，目的地destco；
*   输出：路径数组
****/
static int find_path(int mapblock[][MAPY], int x, int y, MapCoo destco, MapCoo *route)
{
    static int i = 0;
    if(check_pos_valid(mapblock, x, y))
    {
        if(destco.x == x && destco.y == y){
            gValue[x][y] = PASSED;
            route[i].x = x;
            route[i].y = y;
            return 1;
        }
        gValue[x][y] = PASSED;
        route[i].x = x;
        route[i].y = y;
        ++i;
        if(find_path(mapblock, x, y-1, destco, route))
            return 1;

        if(find_path(mapblock, x-1, y, destco, route))
            return 1;

        if(find_path(mapblock, x, y+1, destco, route))
            return 1;

        if(find_path(mapblock, x+1, y, destco, route))
            return 1;
        gValue[x][y] = IMPASSABLE;
        return 0;
    }
    //路径终点标记
    route[i+1].x = ENDROUTE;
    route[i+1].y = ENDROUTE;
    return 0;
}
