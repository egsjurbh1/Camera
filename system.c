/** \brief ϵͳȫ�ֲ�������
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
#include "system.h"
#include "readconfig.h"

/**< ϵͳȫ�ֲ�����ʼ�� */
int SystemInit(SystemPara *sys)
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

    printf("System Parameters Initialize Success.\n");
    return 0;
}








