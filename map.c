/** \brief 道路地图
 *
 * \author lq
 * \update 141126
 * \return 基于坐标
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <windows.h>
#include "map.h"

static int CrossInit(MapFrame *mapf, Cross *cross, int mapx, int mapy);

/**
*   道路地图初始化
*   Input：地图，路口
*/
int RoadMapInit(MapFrame *mapf, Cross *cross, SystemPara *sys)
{
    int i,j,crossid_init;
    int mapx,mapy;

    /* 取系统参数 */
    mapx = sys->Map.x_max;
    mapy = sys->Map.y_max;
    crossid_init = sys->Map.crossid_init;

    /* 地图初始化 */
    for( i = 0; i < mapx; i++)
        for( j = 0; j < mapy; j++) {
            (mapf+mapx*i+j)->coo.x = i;
            (mapf+mapx*i+j)->coo.y = j;
            (mapf+mapx*i+j)->abletag = IMPASSABLE;
        }
    printf("Map Initialize Success.\n");

    /* 交叉路口初始化 */
    //路口一
    cross[0].roadid = crossid_init;
    cross[0].coo.x = 25;
    cross[0].coo.y = 25;
    cross[0].halflength = 25;
    cross[0].halfwidth = 10;
    CrossInit( mapf, &cross[0], mapx, mapy );
    //路口二
    cross[1].roadid = crossid_init + 1;
    cross[1].coo.x = 75;
    cross[1].coo.y = 25;
    cross[1].halflength = 25;
    cross[1].halfwidth = 10;
    CrossInit( mapf, &cross[1], mapx, mapy );

    return 0;
}

/**
*   交叉路口配置函数
*   input:地图，交叉路口
*   output:地图可达性
*   ！Notice：未检测连通性
*/
static int CrossInit(MapFrame *mapf, Cross *cross, int mapx, int mapy)
{
    int i,j;
    MapCoo bcoo1,bcoo2;
    FILE *fp;


    /* 获取交叉路口边界点坐标*/
    bcoo1.x = cross->coo.x - cross->halflength;
    bcoo1.y = cross->coo.y - cross->halfwidth;
    bcoo2.x = cross->coo.x - cross->halfwidth;
    bcoo2.y = cross->coo.y - cross->halflength;

    /* 边界检测 */
    if(bcoo1.x < 0 || bcoo1.y < 0 || bcoo2.x < 0 || bcoo2.y < 0) {
        printf("Road Initialize Failed, Out of Map.\n");
        return -1;
    }

    /* 配置道路可达性 */
    for(i=bcoo1.x;i<=(bcoo1.x+2*cross->halflength);i++)
        for(j=bcoo1.y;j<=(bcoo1.y+2*cross->halfwidth);j++)
            (mapf+mapx*i+j)->abletag = PASSABLE;
    for(i=bcoo2.x;i<=(bcoo2.x+2*cross->halfwidth);i++)
        for(j=bcoo2.y;j<=(bcoo2.y+2*cross->halflength);j++)
            (mapf+mapx*i+j)->abletag = PASSABLE;

    /* 地图输出到文件 */
    fp = fopen("mapdraw.txt", "w+");
    for(j = mapy-1; j >=0 ; j--)
    {
        for(i = 0; i < mapx; i++)
            if((mapf+mapx*i+j)->abletag == IMPASSABLE)
                fprintf(fp,"*");
            else
                fprintf(fp," ");
        fprintf(fp,"\n");
    }
    return 0;
}
