/*
*   道路地图模拟
*   lq
*   update：141028
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "map.h"

static int CrossConfig(int mapb[][MAPY], Cross *cross);

/*
*道路地图初始化
*Input：地图，路口
*/
int RoadMapInit(int mapblock[][MAPY], Cross *cross)
{
    int i,j,ret;

    /* 地图初始化*/
    for( i = 0; i < MAPX; i++)
        for( j = 0; j < MAPY; j++)
            mapblock[i][j] = IMPASSABLE;
    printf("地图初始化完成。\n");

    for(i = 0; i < CROSSNUM; i++)
    {
        /* 配置交叉路口坐标和编号 */
        cross[i].roadid = ROADID+i;
        switch(i){
        case 0:
            cross[i].coo.x = CROSS1_X;
            cross[i].coo.y = CROSS1_Y;
            break;
        case 1:
            cross[i].coo.x = CROSS2_X;
            cross[i].coo.y = CROSS2_Y;
            break;
        default:
            break;
        }

        ret = CrossConfig( mapblock, &cross[i] );
        switch(ret){
        case -2:
            printf("所设路口 %d 和地图已有路面有覆盖！\n",cross[i].roadid);
            break;
        case -1:
            printf("所设路口 %d 坐标越界或道路id不合理！\n",cross[i].roadid);
            break;
        case 0:
            printf("配置路口 %d 成功。\n",cross[i].roadid);
            break;
        default:
            printf("FUCK ERROR!\n");
            break;
        }
    }
    return 0;
}

/*
*交叉路口配置函数
*input:地图，交叉路口
*output:地图可达性
*！Notice：未检测连通性
*/
static int CrossConfig(int mapb[][MAPY], Cross *cross)
{
    /* 获取交叉路口中心周围四个区块坐标*/
    int left_x = cross->coo.x - 1;
    int left_y = cross->coo.y;
    int right_x = cross->coo.x + 1;
    int right_y = cross->coo.y;
    int up_x = cross->coo.x;
    int up_y = cross->coo.y + 1;
    int down_x =cross->coo.x;
    int down_y = cross->coo.y - 1;

    if(left_x >= 0 && left_y >= 0 && right_x >= 0 && right_y >= 0 &&
       up_x >= 0 && up_y >= 0 && down_x >= 0 && down_y >= 0 && cross->roadid != 0)
    {
        if(mapb[cross->coo.x][cross->coo.y] == IMPASSABLE && mapb[left_x][left_y] == IMPASSABLE &&
           mapb[right_x][right_y] == IMPASSABLE &&  mapb[up_x][up_y] == IMPASSABLE &&
           mapb[down_x][down_y] == IMPASSABLE)
        {
            mapb[cross->coo.x][cross->coo.y] = PASSABLE;
            mapb[left_x][left_y] = PASSABLE;
            mapb[right_x][right_y] = PASSABLE;
            mapb[up_x][up_y] = PASSABLE;
            mapb[down_x][down_y] = PASSABLE;
        }
        else
            return -2;  //有覆盖
    }
    else
        return -1;  //道路越界或道路id不合理

    return 0;
}
