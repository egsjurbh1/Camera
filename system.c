/** \brief System Parameters Configuration
 *
 * \author chinglee
 * \update 150101
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
static void defaultconfig( SystemPara *sys );
static void setSyspara_int(FileBuf *buf, char *keyname, int *para, int *res);
static void setSyspara_float(FileBuf *buf, char *keyname, float *para, int *res);

/**< System Initial */
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
    int res = 1;

     /* read config.ini to buf */
    int ret = readfile("config.ini", buf);
    if(ret) {
        printf("readfile error.\n");
        return 1;
    }

    /* prase buf and set syspara */
    ///system
    setSyspara_int(buf, "system_mode", &(sys->system_mode), &res);
    setSyspara_int(buf, "writecycle_cmmu", &(sys->writecycle_cmmu), &res);
    setSyspara_int(buf, "writecycle_instantc", &(sys->writecycle_instantc), &res);
    setSyspara_int(buf, "writecycle_strength", &(sys->writecycle_strength), &res);
    ///Map
    setSyspara_int(buf, "crossnum", &(sys->Map.crossnum), &res);
    setSyspara_int(buf, "crossid_init", &(sys->Map.crossid_init), &res);
    setSyspara_int(buf, "x_max", &(sys->Map.x_max), &res);
    setSyspara_int(buf, "y_max", &(sys->Map.y_max), &res);
    ///Object
    setSyspara_int(buf, "objectnum", &(sys->Object.objectnum), &res);
    setSyspara_int(buf, "objectid_init", &(sys->Object.objectid_init), &res);
    setSyspara_int(buf, "speed0", &(sys->Object.speed[0]), &res);
    setSyspara_int(buf, "speed1", &(sys->Object.speed[1]), &res);
    ///Node
    setSyspara_int(buf, "nodenum", &(sys->Node.nodenum), &res);
    setSyspara_int(buf, "nodeid_init", &(sys->Node.nodeid_init), &res);
    setSyspara_float(buf, "feedbackcost", &(sys->Node.feedbackcost), &res);
    setSyspara_float(buf, "taskcost", &(sys->Node.taskcost), &res);
    setSyspara_float(buf, "strengthinit", &(sys->Node.strengthinit), &res);
    setSyspara_float(buf, "strengthincre", &(sys->Node.strengthincre), &res);
    setSyspara_float(buf, "strengthratio", &(sys->Node.strengthratio), &res);
    setSyspara_float(buf, "pvalue", &(sys->Node.pvalue), &res);
    setSyspara_float(buf, "tvalue", &(sys->Node.tvalue), &res);
    setSyspara_float(buf, "maxvalue", &(sys->Node.maxvalue), &res);
    setSyspara_float(buf, "minvalue", &(sys->Node.minvalue), &res);
    setSyspara_float(buf, "maxload", &(sys->Node.maxload), &res);
    setSyspara_int(buf, "maxnum", &(sys->Node.maxnum), &res);
    setSyspara_int(buf, "fovhalfwidth", &(sys->Node.fovhalfwidth), &res);
    setSyspara_int(buf, "fovlength", &(sys->Node.fovlength), &res);
    setSyspara_int(buf, "commu_mode", &(sys->Node.commu_mode), &res);
    ///Task
    setSyspara_int(buf, "taskid_init", &(sys->Task.taskid_init), &res);
    setSyspara_int(buf, "tasknum", &(sys->Task.tasknum), &res);
    setSyspara_int(buf, "method_type", &(sys->Task.method_type), &res);
    setSyspara_float(buf, "qos_para", &(sys->Task.qos_para), &res);

    if(res)
        return 0;
    else
        return 1;
}

static void defaultconfig( SystemPara *sys )
{
    sys->system_mode = TRACK_MODE;      //ϵͳ����ģʽ
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
    /* Task */
    sys->Task.taskid_init = 10001;      //����ID��ʼ���
    sys->Task.tasknum = 6;              //��������
    sys->Task.method_type = METHOD_TQTA;//TQTA����
    sys->Task.qos_para = 0.9;           //QoS����
}

/**< prasebuffer from buf, write to para */
static void setSyspara_int(FileBuf *buf, char *keyname, int *para, int *res)
{
    int location, ret;

    ret = prasebuffer(keyname, &location, buf);
    if(ret)
        *para = atoi(buf[location].keyvalue);
    else
        printf("prasebuffer error: ", *keyname, "\n");

    *res = (*res) & ret;
}

static void setSyspara_float(FileBuf *buf, char *keyname, float *para, int *res)
{
    int location, ret;

    ret = prasebuffer(keyname, &location, buf);
    if(ret)
        *para = atof(buf[location].keyvalue);
    else
        printf("prasebuffer error: ", *keyname, "\n");

    *res = (*res) & ret;
}
