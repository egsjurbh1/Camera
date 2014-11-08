/*
*   ��·��ͼģ��
*   lq
*   update��141028
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "map.h"

static int CrossConfig(int mapb[][MAPY], Cross *cross);

/*
*��·��ͼ��ʼ��
*Input����ͼ��·��
*/
int RoadMapInit(int mapblock[][MAPY], Cross *cross)
{
    int i,j,ret;

    /* ��ͼ��ʼ��*/
    for( i = 0; i < MAPX; i++)
        for( j = 0; j < MAPY; j++)
            mapblock[i][j] = IMPASSABLE;
    printf("��ͼ��ʼ����ɡ�\n");

    for(i = 0; i < CROSSNUM; i++)
    {
        /* ���ý���·������ͱ�� */
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
            printf("����·�� %d �͵�ͼ����·���и��ǣ�\n",cross[i].roadid);
            break;
        case -1:
            printf("����·�� %d ����Խ����·id������\n",cross[i].roadid);
            break;
        case 0:
            printf("����·�� %d �ɹ���\n",cross[i].roadid);
            break;
        default:
            printf("FUCK ERROR!\n");
            break;
        }
    }
    return 0;
}

/*
*����·�����ú���
*input:��ͼ������·��
*output:��ͼ�ɴ���
*��Notice��δ�����ͨ��
*/
static int CrossConfig(int mapb[][MAPY], Cross *cross)
{
    /* ��ȡ����·��������Χ�ĸ���������*/
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
            return -2;  //�и���
    }
    else
        return -1;  //��·Խ����·id������

    return 0;
}
