/** \brief Object Movement
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
#include "object.h"

static int check_pos_valid(MapFrame *mapf, int x, int y, int mapx, int mapy);
static void MoveModeThread( void *arg );
static int find_direction(Object *obj);
static int moveto( MapFrame *mapf, Object *obj, MapCoo *nextco, int mapx, int mapy );
static void passive_move( Object *obj, MapFrame *mapf, SystemPara *sys );
static void active_move( Object *obj, MapFrame *mapf, Cross *cross, SystemPara *sys );
static void setObject(SystemPara *sys, Object *obj, int mode, int coox, int cooy, int direction, int (*route)[2], int r_size);

/***************************************************************
*   目标初始化函数
*****/
int ObjectInit( Object *obj, SystemPara *sys )
{
    int systemmode = sys->system_mode;
    int route_a[][2] = {{90,25},{75,25},{75,40},{75,25},{25,25},{25,40},{25,25},{10,25},{25,25},{25,10},{25,25},{75,25},{75,10},{75,25},{90,25}};
    int route_b[][2] = {{10,25},{25,25},{25,40},{25,25},{75,25},{75,40},{75,25},{90,25},{75,25},{75,10},{75,25},{25,25},{25,10},{25,25},{10,25}};
    int route_c[][2] = {{10,25},{90,25},{10,25}};
    int route_d[][2] = {{25,10},{25,40},{25,10}};
    int route_e[][2] = {{75,4},{75,46},{75,4}};

    //非跟踪模式
    if( systemmode != TRACK_MODE )
        return 0;

    //设置目标参数
    setObject( sys, obj, PASSIVEMODE, 95, 25, EAST, route_a, sizeof(route_a));
    setObject( sys, obj, PASSIVEMODE, 5, 25, EAST, route_b, sizeof(route_b));
    setObject( sys, obj, PASSIVEMODE, 5, 25, EAST, route_c, sizeof(route_c));
    setObject( sys, obj, PASSIVEMODE, 25, 5, EAST, route_d, sizeof(route_d));
    setObject( sys, obj, PASSIVEMODE, 75, 5, EAST, route_e, sizeof(route_e));

    return 0;
}

/******************************************************************
*   目标运动总控
*   输入：地图,目标信息,控制参数
*   创建目标运动线程
****/
void ObjectMovement( MapFrame *mapf, Cross *cross, Object *obj, SystemPara *sys )
{
    int i, ret, objectnum, systemmode;
    objectnum = sys->Object.objectnum;
    systemmode = sys->system_mode;

    /* 非跟踪模式 */
    if( systemmode != TRACK_MODE)
        return;

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
*   目标运动线程处理函数
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
    int i, m, mapx, mapy;
    int route[MAXROUTE][2];
    MapCoo nextco;

    mapx = sys->Map.x_max;
    mapy = sys->Map.y_max;
    m = (obj->r_size) / sizeof(int) / 2;
    memcpy(route, obj->route, obj->r_size);

    printf("Object:%d, Start POS:(%d, %d)\n", obj->objectid, obj->coo.x, obj->coo.y);
    //路径引导
    for(i = 0; i < m; ++i)
    {
        nextco.x = route[i][0];
        nextco.y = route[i][1];
        if(!moveto( mapf, obj, &nextco, mapx, mapy ))
            break;
        if(i == (m-1))
            i = 0;
    }
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

/**< Set Object */
static void setObject(SystemPara *sys, Object *obj, int mode, int coox, int cooy, int direction, int (*route)[2], int r_size)
{
    int objectnum_init,objectnum;
    static int n = 0;

    objectnum = sys->Object.objectnum;
    objectnum_init = sys->Object.objectid_init;

    if(n > objectnum){
        printf("!WARNING:Object Number Extra.\n");
        return;
    }

    obj[n].mode = mode;
    obj[n].coo.x = coox;
    obj[n].coo.y = cooy;
    obj[n].objectid = objectnum_init + n;
    obj[n].speed = sys->Object.speed[0];
    obj[n].distance = 0;

    switch(mode)
    {
    case PASSIVEMODE:
        obj[n].r_size = r_size;
        memcpy(obj[n].route, route, r_size);
        break;
    case ACTIVEMODE:
        obj[n].direction = direction;
        break;
    default:
        break;
    }

    printf("Object:%d Initialize Success.\n", obj[n].objectid);

    ++n;
}

