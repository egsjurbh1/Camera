/** \brief ��·��ͼ
 *
 * \author lq
 * \update 141126
 * \return ��������
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
*   ��·��ͼ��ʼ��
*   Input����ͼ��·��
*/
int RoadMapInit(MapFrame *mapf, Cross *cross, SystemPara *sys)
{
    int i,j,crossid_init;
    int mapx,mapy;

    /* ȡϵͳ���� */
    mapx = sys->Map.x_max;
    mapy = sys->Map.y_max;
    crossid_init = sys->Map.crossid_init;

    /* ��ͼ��ʼ�� */
    for( i = 0; i < mapx; i++)
        for( j = 0; j < mapy; j++) {
            (mapf+mapx*i+j)->coo.x = i;
            (mapf+mapx*i+j)->coo.y = j;
            (mapf+mapx*i+j)->abletag = IMPASSABLE;
        }
    printf("Map Initialize Success.\n");

    /* ����·�ڳ�ʼ�� */
    //·��һ
    cross[0].roadid = crossid_init;
    cross[0].coo.x = 25;
    cross[0].coo.y = 25;
    cross[0].halflength = 25;
    cross[0].halfwidth = 10;
    CrossInit( mapf, &cross[0], mapx, mapy );
    //·�ڶ�
    cross[1].roadid = crossid_init + 1;
    cross[1].coo.x = 75;
    cross[1].coo.y = 25;
    cross[1].halflength = 25;
    cross[1].halfwidth = 10;
    CrossInit( mapf, &cross[1], mapx, mapy );

    return 0;
}

/**
*   ����·�����ú���
*   input:��ͼ������·��
*   output:��ͼ�ɴ���
*   ��Notice��δ�����ͨ��
*/
static int CrossInit(MapFrame *mapf, Cross *cross, int mapx, int mapy)
{
    int i,j;
    MapCoo bcoo1,bcoo2;
    FILE *fp;


    /* ��ȡ����·�ڱ߽������*/
    bcoo1.x = cross->coo.x - cross->halflength;
    bcoo1.y = cross->coo.y - cross->halfwidth;
    bcoo2.x = cross->coo.x - cross->halfwidth;
    bcoo2.y = cross->coo.y - cross->halflength;

    /* �߽��� */
    if(bcoo1.x < 0 || bcoo1.y < 0 || bcoo2.x < 0 || bcoo2.y < 0) {
        printf("Road Initialize Failed, Out of Map.\n");
        return -1;
    }

    /* ���õ�·�ɴ��� */
    for(i=bcoo1.x;i<=(bcoo1.x+2*cross->halflength);i++)
        for(j=bcoo1.y;j<=(bcoo1.y+2*cross->halfwidth);j++)
            (mapf+mapx*i+j)->abletag = PASSABLE;
    for(i=bcoo2.x;i<=(bcoo2.x+2*cross->halfwidth);i++)
        for(j=bcoo2.y;j<=(bcoo2.y+2*cross->halflength);j++)
            (mapf+mapx*i+j)->abletag = PASSABLE;

    /* ��ͼ������ļ� */
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
