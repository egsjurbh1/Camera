/** \brief ͨ��ϵͳ������
 *
 * \author lq
 * \update 141121
 * \note ���Ӷ������ļ�ȫ�ֲ������ã�δ��ɡ���
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <math.h>
#include "general.h"

static void WriteOutfileThread(void *arg);
static int readfile( char *filename, FileBuf *buf);
static int prasebuffer( const char *keyname, int *location, FileBuf *buf);

/**< ����ܿ� */
void OutputControl( Node *node )
{
    int ret;

    pthread_t file_output;
    pthread_attr_t attr;
    Node *arg;

    //init
    pthread_attr_init(&attr);
    // explicitly specify as joinable PTHREAD_CREATE_JOINABLE or detached  PTHREAD_CREATE_DETACHED
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    arg = node;
    /* ����д�ļ��߳� */
    ret = pthread_create(&file_output, &attr, (void *)WriteOutfileThread, arg);
    if(ret!=0) {
        printf("Create file_output error!\n");
        exit(1);
    }
    //release thread attribute
    pthread_attr_destroy(&attr);
}

/**< д�ļ��߳� */
static void WriteOutfileThread(void *arg)
{
    int i,j;
    time_t it, old_it[2];
    struct tm *timei;
    FILE *fp[2];
    static int tn[2] = {0,0};
    Node *node;

    node = (Node *)arg;

    /* д�ļ� */
    fp[0] = fopen("nodestrength.txt", "w+");
    fp[1] = fopen("communicost.txt", "w+");
    for( i = 0; i < 2; i++)
    {
        fprintf(fp[i], "ϵͳ������\t\n");
        fprintf(fp[i], "NodeNum:\t ObjectNum:\t Strength_Init:\t Strength_Inc:\t Strength_Ratio:\t"
            "������ֵ:\t ��Ϣ����:\t\n %d\t %d\t %.2f\t %.2f\t %.2f\t %.2f\t %.2f\t\n"
            , NODENUM, OBJECTNUM, STRENGTHINIT, STRENGTHINCRE, STRENGTHRATIO, TVALUE, PVALUE);
        fprintf(fp[i], "**************************************************\t\n");
        time(&old_it[i]);
        timei = localtime(&old_it[i]);
        fprintf(fp[i], "%s\t\n", asctime(timei));   //��ǰʱ��
    }
    fprintf(fp[0], "��ϵǿ�Ⱦ���\t\n");//����
    fprintf(fp[1], "ʱ��\tͨ�Ŵ���\t\n");//�����
    fprintf(fp[1], "%d\t%d\t\n", tn[1], count_communication(0));//ͨ�Ŵ���

    while(1)
    {
        time(&it);
        /* д�ڵ��ϵǿ�� */
        if( abs(it - old_it[0]) >= WRITECYCLE_STRENGTH) {
            old_it[0] = it;
            tn[0] += WRITECYCLE_STRENGTH;
            fprintf(fp[0], "%d\t\n", tn[0]);//ʱ������
            printf("Time:%d\t\n", tn[0]);
            for(i = 0; i < NODENUM; i++)
            {
                for(j = 0; j < NODENUM; j++)
                    fprintf(fp[0], "%.1f\t", node[i].rstrength[j]);
                fprintf(fp[0], "\n");
            }
            fprintf(fp[0], "\n");
        }
        /* дͨ�Ŵ��� */
        if( abs(it - old_it[1]) >= WRITECYCLE_COMMU) {
            old_it[1] = it;
            tn[1] += WRITECYCLE_COMMU;
            fprintf(fp[1], "%d\t%d\t\n", tn[1], count_communication(0));//ͨ�Ŵ���
        }
        sleep(1);  //��ʱ
    }
    fclose(fp[0]);
    fclose(fp[1]);
}

/**< ϵͳ��ʼ�� */
int SystemInit(CfgStruct *cfg)
{
    FileBuf buf[CONFIGNUM];
    int ret,location;

    /* �������ļ� */
    ret = readfile("config.ini", buf);
    if(ret) {
        printf("readfile error.\n");
        return -1;
    }
    /* �������� */
    ret = prasebuffer( "nodenum", &location, buf);
    if(!ret)
        cfg->Camera.nodenum = NODENUM;  //��Ĭ�ϲ���
    else
        cfg->Camera.nodenum = atoi(buf[location].keyvalue);

    ret = prasebuffer( "pvalue", &location, buf);
    if(!ret)
        cfg->Camera.pvalue = PVALUE;  //��Ĭ�ϲ���
    else
        cfg->Camera.pvalue = atof(buf[location].keyvalue);

    ret = prasebuffer( "strengthincre", &location, buf);
    if(!ret)
        cfg->Camera.strengthincre = STRENGTHINCRE;  //��Ĭ�ϲ���
    else
        cfg->Camera.strengthincre = atof(buf[location].keyvalue);

    ret = prasebuffer( "strengthinit", &location, buf);
    if(!ret)
        cfg->Camera.strengthinit = STRENGTHINIT;  //��Ĭ�ϲ���
    else
        cfg->Camera.strengthinit = atof(buf[location].keyvalue);

    ret = prasebuffer( "strengthratio", &location, buf);
    if(!ret)
        cfg->Camera.strengthratio = STRENGTHRATIO;  //��Ĭ�ϲ���
    else
        cfg->Camera.strengthratio = atof(buf[location].keyvalue);

    ret = prasebuffer( "tvalue", &location, buf);
    if(!ret)
        cfg->Camera.tvalue = TVALUE;  //��Ĭ�ϲ���
    else
        cfg->Camera.tvalue = atof(buf[location].keyvalue);

    ret = prasebuffer( "objectnum", &location, buf);
    if(!ret)
        cfg->Object.objectnum = OBJECTNUM;  //��Ĭ�ϲ���
    else
        cfg->Object.objectnum = atoi(buf[location].keyvalue);

    return 0;
}

/**<  File Prase Function
*   out:buf
*/
static int readfile( char *filename, FileBuf *buf)
{
	int i,j,k,flag,bannotaion;
	char c;
	FILE *fp;

	i = j = k = flag = bannotaion = 0;
	fp = fopen( filename, "r");
	if(fp == NULL) {
		printf("Open file %s Error!\n", filename);
		return -1;
	}

	while( (c = fgetc(fp)) != EOF )
    {
        if( c == '#' ) {    //ע��
            if(bannotaion)
                bannotaion = 0;
            else
                bannotaion = 1;
            continue;
        }
        if( bannotaion )
            continue;
        if( c == ' ' || c == '\n' || c == '\t')
            continue;
        if( c == '=' ) {
            buf[i].keyname[j] = '\0';
            flag = 1;
            j = 0;
            continue;
        }
        if ( c == ';') {
            buf[i].keyvalue[k] = '\0';
            flag = 0;
            k = 0;
            ++i;
            continue;
        }

        if(flag) {
            buf[i].keyvalue[k] = c;
            ++k;
        } else {
            buf[i].keyname[j] = c;
            ++j;
        }
	}
	fclose(fp);
	return 0;
}

/**< Prase buffer Function
*   find keyname = keyvalue from buf
*/
static int prasebuffer( const char *keyname, int *location, FileBuf *buf)
{
    int i,flag;
    i = flag = 0;
    while(*buf[i].keyname != NULL)
    {
        if( !strcmp(&buf[i].keyname, keyname) ) {
            *location = i;
            flag = 1;
            break;
        }
        else
            ++i;
    }
    if(flag)
        return 1;
    else {
        printf("Cannot find keyname!\n");
        return 0;
    }
}
