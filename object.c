/*
*   �˶�Ŀ��
*   lq
*   update��141031
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <math.h>
#include "object.h"

static int gValue[MAPX][MAPY]; /* ��¼���߹���· */

static int find_path(int mapblock[][MAPY], int x, int y, MapCoo destco, MapCoo *route);
static int check_pos_valid(int mapblock[][MAPY], int x, int y);
static void MoveMode0Thread( void *arg );
static void MoveMode1Thread( void *arg );
static int find_next(int mapblock[][MAPY], int x, int y, MapCoo *nextco );
static void waitdonothing( int timelength );

/***************************************************************
*Ŀ���ʼ������
*****/
int ObjectInit( Object *obj )
{
    int i;
    int objectcoo[OBJECTNUM][2] = {{1,1},{5,1}};  //��ʼ������
    int destcoo[OBJECTNUM][2] = {{5,1},{3,1}};  //Ŀ�ĵ�����
    float speed[2] = {0.5, 0.2};    //Ŀ���ٶ� ��λ����/��

    //��ʼ������
    for( i = 0; i < OBJECTNUM; i++)
    {
        obj[i].coo.x = objectcoo[i][0];
        obj[i].coo.y = objectcoo[i][1];
        obj[i].destcoo.x = destcoo[i][0];
        obj[i].destcoo.y = destcoo[i][1];
        obj[i].objectid = OBJECTID + i;
        obj[i].speed = speed[0];
        obj[i].mode = ACTIVEMODE;
        printf("Ŀ��%d ��ʼ���ɹ���\n", obj[i].objectid );
    }
    return 0;
}

/******************************************************************
*   Ŀ���˶��ܿ�
*   ���룺��ͼ,Ŀ����Ϣ,���Ʋ���
*   ����Ŀ���˶��߳�
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
    /* �̲߳����ӿ� */
    for( i = 0; i < OBJECTNUM; i++ )
    {
        marg1[i].mapblock = mapb;
        marg1[i].obj = &obj[i];
        marg2[i].mapblock = mapb;
        marg2[i].obj = &obj[i];
        mode[i] = obj[i].mode;
    }

    /* �жϿ��Ʋ���*/
    switch(configarg){
    case STOPACTION: //��ֹ
        exit(1);
        break;
    case EXECUTEACTION: //ִ��
        break;
    default:
        exit(1);
        break;
    }

    /* �ж��˶���ʽ */
    for( i = 0; i < OBJECTNUM; i++ )
    {
        switch(mode[i]){
        case PASSIVEMODE: //����·������ģʽ
            ret = pthread_create(&objectthread[i], &attr, (void *)MoveMode0Thread, &marg1[i]);    //mode0�˶��߳�
            if(ret!=0) {
                printf("Create MoveMode0Thread error!\n");
                exit(1);
            }
            printf("Ŀ�� %d �˶��̣߳�ģʽ:0��·��������������\n", obj[i].objectid );
            break;
        case ACTIVEMODE: //����Ѱ·ģʽ
            ret = pthread_create(&objectthread[i], &attr, (void *)MoveMode1Thread, &marg2[i]);    //mode0�˶��߳�
            if(ret!=0) {
                printf("Create MoveMode1Thread error!\n");
                exit(1);
            }
            printf("Ŀ�� %d �˶��̣߳�ģʽ1�����Ѱ·��������\n", obj[i].objectid );
            break;
        default:
            break;
        }
    }
    //release thread attribute
    pthread_attr_destroy(&attr);
}

/*************************************
*   Ŀ���˶��߳�1���ڵ������������Ѱ·��
*   ���룺Ŀ����Ϣ,���,�ٶ�(n��/s)
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

    //��������
    psarg = (MoveArg *)arg;
    mapblock = psarg->mapblock;
    obj = psarg->obj;

    speed = obj->speed;

    printf("Ŀ�� %d ��ʼλ�� (%d, %d)\n", obj->objectid, obj->coo.x, obj->coo.y);
    while(1)
    {
        //Ŀ����ʼ����
        currentco.x = obj->coo.x;
        currentco.y = obj->coo.y;
        //Ѱ����һ����
        if( find_next( mapblock, currentco.x, currentco.y, &nextco ) )
        {
            obj->coo.x = nextco.x;  //�ƶ�
            obj->coo.y = nextco.y;
            printf("Ŀ�� %d λ�� (%d, %d)\n", obj->objectid, obj->coo.x, obj->coo.y);
            waitdonothing(1/speed); //�ȴ�
        }
        //usleep(100);
    }
}

/*****
*   Ŀ���ƶ�����һ����
*   ���룺��ͼ���������x��y��Ŀ�ꣻ
*   ����1�ҵ���ͨ�е�
*****/
static int find_next(int mapblock[][MAPY], int x, int y, MapCoo *nextco )
{
    int i;
    /* ���������һ��λ */
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
*   �ȴ�����
*   ��λ��
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
*   Ŀ���˶��߳�0������Ѱ·������·�������ڵ㣩
*   ���룺Ŀ����Ϣ,���,�յ�,�ٶ�(n��/s)
*
*****************************/
static void MoveMode0Thread( void *arg )
{
    MoveArg *psarg;//�ӿڽṹ
    int **mapblock;
    Object *obj;

    MapCoo destco;
    float speed;
    int i = 0;
    MapCoo startco;
    MapCoo route[20];
    //��������
    psarg = (MoveArg *)arg;
    mapblock = psarg->mapblock;
    obj = psarg->obj;
    //Ŀ������
    startco.x = obj->coo.x;
    startco.y = obj->coo.y;
    destco.x = obj->destcoo.x;
    destco.y = obj->destcoo.y;
    speed = obj->speed;
    //Ѱ��·��
    find_path( mapblock, startco.x, startco.y, destco, route);
    //Ŀ�갴·��������ƶ����յ�
    printf("Ŀ�� %d ��ʼλ�� (%d, %d)\n", obj->objectid, obj->coo.x, obj->coo.y);
    while(1)
    {
        sleep(1);
        if(route[i].x == ENDROUTE)
            break;
        waitdonothing(1/speed); //�ȴ�
        ++i;
        obj->coo.x = route[i].x;
        obj->coo.y = route[i].y;
        printf("Ŀ�� %d λ�� (%d, %d)\n", obj->objectid, obj->coo.x, obj->coo.y);
    }
}

/******�߽���******/
static int check_pos_valid(int mapblock[][MAPY], int x, int y)
{
    /* �ڵ��Ƿ���߽� */
    if(x < 0 || x > MAPX || y < 0 || y > MAPY)
        return 0;

    /* ��ǰ�ڵ��Ƿ����· */
    if(mapblock[x][y] == IMPASSABLE)
        return 0;

    /* ��ǰ�ڵ��Ƿ��Ѿ��߹� */
    if(gValue[x][y] == PASSED)
        return 0;

    return 1;
}

/****
*   Ѱ�ҿ�ͨ��·��
*   ���룺��ͼ���������x��y��Ŀ�ĵ�destco��
*   �����·������
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
    //·���յ���
    route[i+1].x = ENDROUTE;
    route[i+1].y = ENDROUTE;
    return 0;
}
