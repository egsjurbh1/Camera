/** \brief ���
 *
 * \author lq
 * \update 141202
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
#include "general.h"
#include "system.h"

static void WriteOutfileThread(void *arg);
static void writeoutfile(Node *node, Object *obj, SystemPara *sys);
static int count_utility(float *ic, Node *node, int nodenum);

/**< ����ܿ� */
void OutputControl( Node *node, Object *obj, SystemPara *sys )
{
    int ret;

    pthread_t file_output;
    pthread_attr_t attr;
    FileThreadArg filearg;

    //init
    pthread_attr_init(&attr);
    // explicitly specify as joinable PTHREAD_CREATE_JOINABLE or detached  PTHREAD_CREATE_DETACHED
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    filearg.node = node;
    filearg.object = obj;
    filearg.sys = sys;
    /* ����д�ļ��߳� */
    ret = pthread_create(&file_output, &attr, (void *)WriteOutfileThread, &filearg);
    if(ret != 0) {
        printf("Create file_output error!\n");
        exit(1);
    }
    sleep(1);
    printf("WriteOutFile Thread Created.\n");
    //release thread attribute
    pthread_attr_destroy(&attr);
}

/**< д�ļ��߳� */
static void WriteOutfileThread(void *arg)
{
    //�ӿڲ���
    FileThreadArg *psarg;
    Node *node;
    Object *obj;
    SystemPara *sys;

    //��������
    psarg = (FileThreadArg *)arg;
    obj = psarg->object;
    node = psarg->node;
    sys = psarg->sys;

    //д�ļ�
    writeoutfile(node, obj, sys);
}

/**< д�ļ������� */
static void writeoutfile(Node *node, Object *obj, SystemPara *sys)
{
    int i,j;
    int nodenum, writecycle_instantc, writecycle_strength, writecycle_commu;
    /* ȡϵͳ���� */
    nodenum = sys->Node.nodenum;
    writecycle_instantc = sys->writecycle_instantc;
    writecycle_strength = sys->writecycle_strength;
    writecycle_commu = sys->writecycle_cmmu;

    time_t it[ONUM], old_it[ONUM];
    float cc,utility;
    struct tm *timei;
    FILE *fp[ONUM];
    FILE *fps[nodenum];
    char filename[FILEBUFFER];
    char title[FILEBUFFER];
    static int tn[ONUM];
    static int nf = 1;

    memset(tn, 0, sizeof(tn[ONUM]));
    /* ���ļ� */
    fp[0] = fopen("nodestrength.txt", "w+");
    fp[1] = fopen("communicost.txt", "w+");
    fp[2] = fopen("instantcommunicost.txt", "w+");
    fp[3] = fopen("utility.txt", "w+");
    for( i = 0; i < nodenum; i++) {
        sprintf(filename,"C%d.txt",(i+1));
        fps[i] = fopen(filename, "w+");
    }
    for( i = 0; i < ONUM; i++)
    {
        fprintf(fp[i], "ϵͳ������\t\n");
        fprintf(fp[i], "NodeNum:\t ObjectNum:\t Strength_Init:\t Strength_Inc:\t Strength_Ratio:\t"
            "������ֵ:\t ��Ϣ����:\t ͨ�ŷ�ʽ:\t\n %d\t %d\t %.2f\t %.2f\t %.2f\t %.2f\t %.2f\t %d\t\n"
            , nodenum, sys->Object.objectnum, sys->Node.strengthinit,
            sys->Node.strengthincre, sys->Node.strengthratio, sys->Node.tvalue,
            sys->Node.pvalue, sys->Node.commu_mode);
        fprintf(fp[i], "**************************************************\t\n");
        time(&old_it[i]);
        timei = localtime(&old_it[i]);
        fprintf(fp[i], "%s\t\n", asctime(timei));   //��ǰʱ��
    }
    for( i = 0; i < nodenum; i++) {
        fprintf(fps[i], "STEP\t");//����
        for( j = 0; j < nodenum; ++j) {
            sprintf(title, "C%d\t", (j+1));
            fprintf(fps[i], title);
        }
        fprintf(fps[i], "\n");
    }
    fprintf(fp[0], "��ϵǿ�Ⱦ���\t\n");//����
    fprintf(fp[1], "ʱ��\tͨ�Ŵ���\t\n");//�����
    fprintf(fp[1], "%d\t%.1f\t\n", tn[1], count_communication(0,0,&cc,0,0));//ͨ�Ŵ���
    fprintf(fp[2], "ʱ��\t˲ʱͨ�Ŵ���\t\n");//�����
    fprintf(fp[2], "%d\t%.1f\t\n", tn[2], count_communication(0,0,&cc,0,0));//ͨ�Ŵ���
    fprintf(fp[3], "ʱ��\t˲ʱЧ��\t\n");//�����
    count_utility(&utility, node, nodenum);
    fprintf(fp[3], "%d\t%.1f\t\n", tn[3], utility);//Ч��

    while(1)
    {
        /* д˲ʱͨ���� */
        time(&it[2]);
        if( abs(it[2] - old_it[2]) >= writecycle_instantc) {
            old_it[2] = it[2];
            tn[2] += writecycle_instantc;
            count_communication(OUTPUTIC,0,&cc,0,0);
            fprintf(fp[2], "%d\t%.1f\t\n", tn[2], cc);//˲ʱͨ����
        }

        /* д˲ʱUtility */
        time(&it[3]);
        if( abs(it[3] - old_it[3]) >= writecycle_instantc) {
            old_it[3] = it[3];
            tn[3] += writecycle_instantc;
            count_utility(&utility, node, nodenum);
            fprintf(fp[3], "%d\t%.3f\t\n", tn[3], utility);//Ч��
            /*for(k = 0; k < nodenum; k++)
                fprintf(fp[3], "%.3f\t", node[k].utility);//Ч��*/
        }

        /* д�ڵ��ϵǿ�� */
        time(&it[0]);
        if( abs(it[0] - old_it[0]) >= writecycle_strength) {
            old_it[0] = it[0];
            tn[0] += writecycle_strength;
            fprintf(fp[0], "%d\t\n", tn[0]);//ʱ������
            printf("Time:%d\t\n", tn[0]);
            for(i = 0; i < nodenum; i++)
            {
                fprintf(fps[i], "%d\t", nf);
                for(j = 0; j < nodenum; j++) {
                    fprintf(fp[0], "%.1f\t", node[i].rstrength[j]);
                    fprintf(fps[i], "%.1f\t", node[i].rstrength[j]);
                }
                fprintf(fp[0], "\n");
                fprintf(fps[i], "\n");
            }
            ++nf;
            fprintf(fp[0], "\n");
        }

        /* дͨ�Ŵ��� */
        time(&it[1]);
        if( abs(it[1] - old_it[1]) >= writecycle_commu) {
            old_it[1] = it[1];
            tn[1] += writecycle_commu;
            fprintf(fp[1], "%d\t%.1f\t\n", tn[1], count_communication(0,0,&cc,0,0));//ͨ�Ŵ���
        }
        sleep(1);  //��ʱ
    }

    for(i = 0; i < ONUM; i++)
        fclose(fp[i]);
    for(i = 0; i < nodenum; i++)
        fclose(fps[i]);
}

/*****Utility*****/
/**
 */
static int count_utility(float *ic, Node *node, int nodenum)
{
    int i;
    *ic = 0;
    for(i = 0; i < nodenum; i++)
        *ic += node[i].utility;
    return 0;
}



