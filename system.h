#ifndef _SYSTEM_H
#define _SYSTEM_H

#define sleep(x) Sleep(1000 * x)    //Sleep���� sleep�� ��ͬϵͳ���ܲ���sleep

#define MULTICAST       1       //���񷢲�ģʽ���鲥
#define BROADCAST       2       //���񷢲�ģʽ���㲥
#define NONEID          0       //�ޱ��
#define BUFFER          10      //buffer

/* Map�����ṹ�� */
typedef struct MapParametersStruct
{
    int x_max;              //��ͼĬ��x���������
    int y_max;              //��ͼĬ��y���������
    int crossnum;           //·������
    int crossid_init;       //·��ID��ʼ���
}MapPara;

/* Object�����ṹ�� */
typedef struct ObjectParametersStruct
{
    int objectnum;                  //Ŀ������
    int objectid_init;              //Ŀ��ID��ʼ���
    int speed[BUFFER];              //Ŀ���ٶ�
}ObjectPara;

/* Node�����ṹ�� */
typedef struct NodeParametersStruct
{
    int nodenum;            //�ڵ�����
    int nodeid_init;        //�ڵ�ID��ʼ���
    int maxnum;             //�ڵ�������Ŀ����
    float maxload;          //�ڵ㸺����ֵ
    int commu_mode;         //�ڵ�ͨ�ŷ�ʽ
    int fovlength;          //���򳤶�
    int fovhalfwidth;       //������
    float strengthinit;     //strength��ֵ
    float strengthincre;    //strength����
    float strengthratio;    //strengthϵ��
    float pvalue;           //��ʽ�������ʦ�
    float tvalue;           //��ʽ������ֵ
    float maxvalue;         //strength���ֵ
    float minvalue;         //strength��Сֵ
    float taskcost;         //������Ϣͨ��Ȩ��
    float feedbackcost;     //������Ϣͨ��Ȩ��
}NodePara;

/* ϵͳ�����ṹ�� */
typedef struct SystemParametersStruct
{
    int writecycle_cmmu;        //ͳ��ͨ�Ŵ��۵�д�����ڣ��룩
    int writecycle_strength;    //�ڵ�ǿ��д�����ڣ��룩
    int writecycle_instantc;    //˲ʱͨ��д�����ڣ��룩
    MapPara Map;
    ObjectPara Object;
    NodePara Node;
}SystemPara;

extern int SystemInit( SystemPara *sys);                 //ϵͳ������ʼ��

#endif // _SYSTEM_H

