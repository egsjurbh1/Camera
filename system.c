/** \brief ϵͳȫ�ֲ�������
 *
 * \author lq
 * \update 141204
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
#include "system.h"
#include "readconfig.h"

static int fileconfig( SystemPara *sys );
static int defaultconfig( SystemPara *sys );

/**< ϵͳȫ�ֲ�����ʼ�� */
int SystemInit(SystemPara *sys)
{
    int ret;

    ret = fileconfig( sys );
    if(ret) {
        printf("fileconfig failed!read defaultconfig.\n");
        defaultconfig( sys );
    }

    printf("System Parameters Initialize Success.\n");
    return 0;
}

static int fileconfig( SystemPara *sys )
{
    FileBuf buf[CONFIGNUM];
    int i,ret[CONFIGNUM],location,res;

     /* �������ļ� */
    res = readfile("config.ini", buf);
    if(res) {
        printf("readfile error.\n");
        return -1;
    }
    res = 1;
    /* File */
    ret[0] = prasebuffer( "writecycle_cmmu", &location, buf);
    if(ret[0])
        sys->writecycle_cmmu = atoi(buf[location].keyvalue);
    ret[1] = prasebuffer( "writecycle_instantc", &location, buf);
    if(ret[1])
        sys->writecycle_instantc = atoi(buf[location].keyvalue);
    ret[2] = prasebuffer( "writecycle_strength", &location, buf);
    if(ret[2])
        sys->writecycle_strength = atoi(buf[location].keyvalue);

    /* Map */
    ret[3] = prasebuffer( "crossnum", &location, buf);
    if(ret[3])
        sys->Map.crossnum = atoi(buf[location].keyvalue);
    ret[4] = prasebuffer( "crossid_init", &location, buf);
    if(ret[4])
        sys->Map.crossid_init = atoi(buf[location].keyvalue);
    ret[5] = prasebuffer( "x_max", &location, buf);
    if(ret[5])
        sys->Map.x_max = atoi(buf[location].keyvalue);
    ret[6] = prasebuffer( "y_max", &location, buf);
    if(ret[6])
        sys->Map.y_max = atoi(buf[location].keyvalue);

    /* Object */
    ret[7] = prasebuffer( "objectnum", &location, buf);
    if(ret[7])
        sys->Object.objectnum = atoi(buf[location].keyvalue);
    ret[8] = prasebuffer( "objectid_init", &location, buf);
    if(ret[8])
        sys->Object.objectid_init = atoi(buf[location].keyvalue);
    ret[9] = prasebuffer( "speed0", &location, buf);
    if(ret[9])
        sys->Object.speed[0] = atof(buf[location].keyvalue);
    ret[10] = prasebuffer( "speed1", &location, buf);
    if(ret[10])
        sys->Object.speed[1] = atof(buf[location].keyvalue);

    /* Node */
    ret[11] = prasebuffer( "nodenum", &location, buf);
    if(ret[11])
        sys->Node.nodenum = atoi(buf[location].keyvalue);
    ret[12] = prasebuffer( "nodeid_init", &location, buf);
    if(ret[12])
        sys->Node.nodeid_init = atoi(buf[location].keyvalue);
    ret[13] = prasebuffer( "feedbackcost", &location, buf);
    if(ret[13])
        sys->Node.feedbackcost = atof(buf[location].keyvalue);
    ret[14] = prasebuffer( "taskcost", &location, buf);
    if(ret[14])
        sys->Node.taskcost = atof(buf[location].keyvalue);
    ret[15] = prasebuffer( "strengthinit", &location, buf);
    if(ret[15])
        sys->Node.strengthinit = atof(buf[location].keyvalue);
    ret[16] = prasebuffer( "strengthincre", &location, buf);
    if(ret[16])
        sys->Node.strengthincre = atof(buf[location].keyvalue);
    ret[17] = prasebuffer( "strengthratio", &location, buf);
    if(ret[17])
        sys->Node.strengthratio = atof(buf[location].keyvalue);
    ret[18] = prasebuffer( "pvalue", &location, buf);
    if(ret[18])
        sys->Node.pvalue = atof(buf[location].keyvalue);
    ret[19] = prasebuffer( "tvalue", &location, buf);
    if(ret[19])
        sys->Node.tvalue = atof(buf[location].keyvalue);
    ret[20] = prasebuffer( "maxvalue", &location, buf);
    if(ret[20])
        sys->Node.maxvalue = atof(buf[location].keyvalue);
    ret[21] = prasebuffer( "minvalue", &location, buf);
    if(ret[21])
        sys->Node.minvalue = atof(buf[location].keyvalue);
    ret[22] = prasebuffer( "maxnum", &location, buf);
    if(ret[22])
        sys->Node.maxnum = atoi(buf[location].keyvalue);
    ret[23] = prasebuffer( "maxload", &location, buf);
    if(ret[23])
        sys->Node.maxload = atof(buf[location].keyvalue);
    ret[24] = prasebuffer( "fovhalfwidth", &location, buf);
    if(ret[24])
        sys->Node.fovhalfwidth = atoi(buf[location].keyvalue);
    ret[25] = prasebuffer( "fovlength", &location, buf);
    if(ret[25])
        sys->Node.fovlength = atoi(buf[location].keyvalue);
    ret[26] = prasebuffer( "commu_mode", &location, buf);
    if(ret[26])
        sys->Node.commu_mode = atoi(buf[location].keyvalue);

    for(i = 0; i < 27; i++)
        res = res & ret[i];

    if(res)
        return 0;
    else
        return 1;
}

static int defaultconfig( SystemPara *sys )
{
    /* File */
    sys->writecycle_cmmu = 15;          //ͳ��ͨ�Ŵ��۵�д�����ڣ��룩
    sys->writecycle_instantc = 1;       //˲ʱͨ��д�����ڣ��룩
    sys->writecycle_strength = 3;       //�ڵ�ǿ��д�����ڣ��룩
    /* Map */
    sys->Map.crossnum = 2;              //·������
    sys->Map.crossid_init = 901;        //·��ID��ʼ���
    sys->Map.x_max = 100;               //��ͼĬ��x���������
    sys->Map.y_max = 100;               //��ͼĬ��y���������
    /* Object */
    sys->Object.objectnum = 4;          //Ŀ������
    sys->Object.objectid_init = 601;    //Ŀ��ID��ʼ���
    sys->Object.speed[0] = 5;           //Ŀ���ٶȼ�������/��
    sys->Object.speed[1] = 2;
    /* Node */
    sys->Node.nodenum = 6;              //�ڵ�����
    sys->Node.nodeid_init = 101;        //�ڵ�ID��ʼ���
    sys->Node.feedbackcost = 0.1;       //������Ϣͨ��Ȩ��
    sys->Node.taskcost = 0.9;           //������Ϣͨ��Ȩ�أ���Ϊ1
    sys->Node.strengthinit = 1.3;       //strength��ֵ
    sys->Node.strengthincre = 1.0;      //strength����
    sys->Node.strengthratio = 0.1;      //strengthϵ��
    sys->Node.pvalue = 0.01;            //��ʽ�������ʦ�
    sys->Node.tvalue = 1.0;             //��ʽ������ֵ
    sys->Node.maxvalue = 5.0;           //strength���ֵ
    sys->Node.minvalue = 0.0;           //strength��Сֵ
    sys->Node.maxnum = 30;              //�ڵ�������Ŀ����
    sys->Node.maxload = 0.95;           //�ڵ㸺����ֵ
    sys->Node.fovhalfwidth = 5;         //������
    sys->Node.fovlength = 10;           //���򳤶�
    sys->Node.commu_mode = MULTICAST;   //ͨ�ŷ�ʽ

    return 0;
}








