/** \brief 运动目标
 *
 * \author lq
 * \update 141201
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
#include "object.h"

static int check_pos_valid(MapFrame *mapf, int x, int y, int mapx, int mapy);
static void MoveModeThread( void *arg );
static int find_direction(Object *obj);
static void waitdonothing( float timelength );
static int moveto( MapFrame *mapf, Object *obj, MapCoo *nextco, int mapx, int mapy );
static int route(int mode, Object *obj, MapFrame *mapf, int mapx, int mapy);
static void passive_move( Object *obj, MapFrame *mapf, SystemPara *sys );
static void active_move( Object *obj, MapFrame *mapf, Cross *cross, SystemPara *sys );

/***************************************************************
*   目标初始化函数
*****/
int ObjectInit( Object *obj, SystemPara *sys )
{
    int i;
    int objectnum_init,objectnum;
    objectnum = sys->Object.objectnum;
    objectnum_init = sys->Object.objectid_init;

    //初始化参数
    for( i = 0; i < objectnum; i++)
    {
        obj[i].objectid = objectnum_init + i;
        obj[i].speed = sys->Object.speed[0];
        obj[i].distance = 0;
        printf("Object:%d Initialize Success.\n", obj[i].objectid );
    }
    obj[0].mode = PASSIVEMODE;      //PASSIVEMODE, ACTIVEMODE
    obj[0].coo.x = 90;
    obj[0].coo.y = 25;
    obj[0].direction = EAST;
    obj[0].route = RMODE_A;
    obj[1].mode = PASSIVEMODE;      //PASSIVEMODE, ACTIVEMODE
    obj[1].coo.x = 70;
    obj[1].coo.y = 25;
    obj[1].route = RMODE_B;
    obj[2].mode = PASSIVEMODE;      //PASSIVEMODE, ACTIVEMODE
    obj[2].coo.x = 25;
    obj[2].coo.y = 10;
    obj[2].route = RMODE_C;
    obj[3].mode = ACTIVEMODE;      //PASSIVEMODE, ACTIVEMODE
    obj[3].coo.x = 90;
    obj[3].coo.y = 25;
    obj[3].direction = EAST;
    return 0;
}

/******************************************************************
*   目标运动总控
*   输入：地图,目标信息,控制参数
*   创建目标运动线程
****/
void ObjectMovement( MapFrame *mapf, Cross *cross, Object *obj, SystemPara *sys )
{
    int i, ret, objectnum;
    objectnum = sys->Object.objectnum;

    pthread_t objectthread[objectnum];
    pthread_attr_t attr;
    MoveArg marg[objectnum];

    //init
    pthread_attr_init(&attr);
    // explicitly specify as joinable PTHREAD_CREATE_JOINABLE or detached  PTHREAD_CREATE_DETACHED
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    /* 线程参数接口 */
    for( i = 0; i < objectnum; i++ )
    {
        marg[i].mapf = mapf;
        marg[i].obj = &obj[i];
        marg[i].cross = cross;
        marg[i].sys = sys;
    }

    /* 创建各目标线程 */
    for( i = 0; i < objectnum; i++ )
    {
        ret = pthread_create(&objectthread[i], &attr, (void *)MoveModeThread, &marg[i]);    //mode0运动线程
        if(ret) {
            printf("Create MoveModeThread of %d error!\n", obj[i].objectid);
            exit(1);
        }
        sleep(1);
        printf("Object:%d ObjectMovement Thread Created. Mode:%d\n", obj[i].objectid, obj[i].mode );
    }
    //release thread attribute
    pthread_attr_destroy(&attr);
}

/*************************************
*   目标运动线程
*************************************/
static void MoveModeThread( void *arg )
{
    int mode;
    //接口参数
    MoveArg *psarg;
    MapFrame *mapf;
    Object *obj;
    Cross *cross;
    SystemPara *sys;

    //参数引用
    psarg = (MoveArg *)arg;
    mapf = psarg->mapf;
    obj = psarg->obj;
    cross = psarg->cross;
    sys = psarg->sys;

    mode = obj->mode;
    switch(mode){
    case ACTIVEMODE:
        active_move(obj, mapf, cross, sys);//主动寻路
        break;
    case PASSIVEMODE:
        passive_move(obj, mapf, sys);//被动引导
        break;
    default:
        break;
    }
}

/** \brief 主动寻路模式
 *
 * \param 目标信息,起点,速度
 * \param
 * \return
 *
 */
 static void active_move( Object *obj, MapFrame *mapf, Cross *cross, SystemPara *sys )
 {
    int i,crossnum,mapx,mapy;
    float speed;
    MapCoo nextco;

    printf("Object:%d, Start POS:(%d, %d)\n", obj->objectid, obj->coo.x, obj->coo.y);
    speed = obj->speed;
    nextco.x = cross[1].coo.x;
    nextco.y = cross[1].coo.y;
    crossnum = sys->Map.crossnum;
    mapx = sys->Map.x_max;
    mapy = sys->Map.y_max;
    //移动到路口中心
    moveto(mapf, obj, &nextco, mapx, mapy);
    while(1)
    {
        /* 判定是否到达路口中心决策点 */
        for(i = 0; i < crossnum; ++i) {
            if(obj->coo.x == cross[i].coo.x && obj->coo.y == cross[i].coo.y) {
                find_direction(obj);
                printf("Object:%d, CrossCenter POS:(%d, %d), D:%d\n", obj->objectid, obj->coo.x, obj->coo.y, obj->direction);
                sleep(1/speed);//等待
                break;
            }
        }
        /* 根据方向移动到下一位置 */
        switch(obj->direction){
        case NORTH:
            ++obj->coo.y;
            if(!check_pos_valid(mapf, obj->coo.x, obj->coo.y, mapx, mapy)) {
                --obj->coo.y;
                obj->direction = SOUTH; //change move direction
                printf("Object:%d, MapBoundary POS:(%d, %d), D:↓\n", obj->objectid, obj->coo.x, obj->coo.y);
            }
            break;
        case SOUTH:
            --obj->coo.y;
            if(!check_pos_valid(mapf, obj->coo.x, obj->coo.y, mapx, mapy)) {
                ++obj->coo.y;
                obj->direction = NORTH; //change move direction
                printf("Object:%d, MapBoundary POS:(%d, %d), D:↑\n", obj->objectid, obj->coo.x, obj->coo.y);
            }
            break;
        case EAST:
            ++obj->coo.x;
            if(!check_pos_valid(mapf, obj->coo.x, obj->coo.y, mapx, mapy)) {
                --obj->coo.x;
                obj->direction = WEST; //change move direction
                printf("Object:%d, MapBoundary POS:(%d, %d), D:←\n", obj->objectid, obj->coo.x, obj->coo.y);
            }
            break;
        case WEST:
            --obj->coo.x;
            if(!check_pos_valid(mapf, obj->coo.x, obj->coo.y, mapx, mapy)) {
                ++obj->coo.x;
                obj->direction = EAST; //change move direction
                printf("Object:%d, MapBoundary POS:(%d, %d), D:→\n", obj->objectid, obj->coo.x, obj->coo.y);
            }
            break;
        default:
            break;
        }
        sleep(1/speed);//等待
    }
}

/*****
*   决策下一方位
*   输入：地图，起点坐标x、y，目标；
*   返回1找到可通行点
*****/
static int find_direction(Object *obj)
{
    int i;
    /* 随机决策下一方位 */
    srand((unsigned)time(NULL));
    i = rand() % PERCENT;

    switch(i){
    case 0:
        obj->direction = EAST;
        ++obj->coo.x;
        break;
    case 1:
        obj->direction = WEST;
        --obj->coo.x;
        break;
    case 2:
        obj->direction = NORTH;
        ++obj->coo.y;
        break;
    case 3:
        obj->direction = SOUTH;
        --obj->coo.y;
        break;
    default:
        printf("FUCK ERROR!\n");
        return 0;
    }

    return 1;
}

/** \brief 路径引导模式
 *
 * \param 目标信息,起点,终点,速度
 * \param
 * \return
 *
 */
static void passive_move( Object *obj, MapFrame *mapf, SystemPara *sys )
{
    int routemode;
    int mapx,mapy;

    routemode = obj->route;
    mapx = sys->Map.x_max;
    mapy = sys->Map.y_max;

    printf("Object:%d, Start POS:(%d, %d)\n", obj->objectid, obj->coo.x, obj->coo.y);
    //路径引导
    if(route(routemode, obj, mapf, mapx, mapy)) {
        printf("Error:Object %d route failed.\n", obj->objectid);
        exit(1);
    }
}

/** \brief Route路径函数
 *
 * \param
 * \param
 * \return
 *
 */
static int route(int mode, Object *obj, MapFrame *mapf, int mapx, int mapy)
{
    int i,m;
    int mc_a[9][2] = {{75,25},{25,25},{25,48},{25,25},{1,25},{25,25},{25,1},{25,25},{75,25}};
    int mc_b[3][2] = {{75,25},{1,25},{75,25}};
    int mc_c[3][2] = {{25,1},{25,49},{25,1}};
    MapCoo nextco;

    switch(mode){
    case RMODE_A:
        m = sizeof(mc_a)/sizeof(int)/2;
        for(i = 0; i < m; ++i) {
            nextco.x = mc_a[i][0];
            nextco.y = mc_a[i][1];
            if(!moveto( mapf, obj, &nextco, mapx, mapy ))
                break;
            if(i == (m-1))
                i = 0;
        }
        break;
    case RMODE_B:
        m = sizeof(mc_b)/sizeof(int)/2;
        for(i = 0; i < m; ++i) {
            nextco.x = mc_b[i][0];
            nextco.y = mc_b[i][1];
            if(!moveto( mapf, obj, &nextco, mapx, mapy ))
                break;
            if(i == (m-1))
                i = 0;
        }
        break;
     case RMODE_C:
        m = sizeof(mc_c)/sizeof(int)/2;
        for(i = 0; i < m; ++i) {
            nextco.x = mc_c[i][0];
            nextco.y = mc_c[i][1];
            if(!moveto( mapf, obj, &nextco, mapx, mapy ))
                break;
            if(i == (m-1))
                i = 0;
        }
        break;
    default:
        break;
    }
    return 0;
}

/** \brief Moveto定点移动函数
 *
 * \param (1,2)->(3,6) 移动过程：(1,2)->(3,2)->(3,6)
 * \param
 * \return
 *
 */
static int moveto( MapFrame *mapf, Object *obj, MapCoo *nextco, int mapx, int mapy )
{
    int nx,ny,ox,oy;
    nx = nextco->x;
    ny = nextco->y;
    ox = obj->coo.x;
    oy = obj->coo.y;
    int speed = obj->speed;

    if(!check_pos_valid(mapf, nx, ny, mapx, mapy)) {
        printf("(%d,%d) Out of MAP or impassable.\n", nx, ny);
        return 0;
    }
    //水平方向
    while(abs(obj->coo.x - nx) != 0 )
    {
        if(check_pos_valid(mapf, obj->coo.x, obj->coo.y, mapx, mapy)) {
            if(nx >= ox)
                ++obj->coo.x;
            else
                --obj->coo.x;
            sleep(1/speed);//等待
        }
        else {
            printf("Bad start position.\n");
            break;
        }
    }
    //竖直方向
    while(abs(obj->coo.y - ny) != 0 )
    {
        if(check_pos_valid(mapf, obj->coo.x, obj->coo.y, mapx, mapy)) {
            if(ny >= oy)
                ++obj->coo.y;
            else
                --obj->coo.y;
            sleep(1/speed);//等待
        }
        else {
            printf("Bad start position.\n");
            break;
        }
    }

    printf("Object:%d, Position:(%d, %d)\n", obj->objectid, obj->coo.x, obj->coo.y);
    return 1;
}

/******边界检测******/
static int check_pos_valid(MapFrame *mapf, int x, int y, int mapx, int mapy)
{
    /* 节点是否出边界 */
    if(x < 0 || x > mapx || y < 0 || y > mapy)
        return 0;

    /* 当前节点是否存在路 */
    if((mapf+x*mapx+y)->abletag == IMPASSABLE)
        return 0;

    return 1;
}

/*******************************************
*   等待函数
*   单位秒
*****/
static void waitdonothing( float timelength )
{
    clock_t tb,te;

    tb = clock();
    while(1)
    {
        te = clock();
        if( abs(te - tb)/CLOCKS_PER_SEC >= timelength )
            break;
        Sleep(50);
    }
}
