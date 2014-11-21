#ifndef _GENERAL_H
#define _GENERAL_H

#include "camera.h"
#include "object.h"
#include "map.h"

#define WRITECYCLE_COMMU    15      //ͳ��ͨ�Ŵ��۵�д�����ڣ��룩
#define WRITECYCLE_STRENGTH 3       //�ڵ�ǿ��д�����ڣ��룩

#define BUFSIZE             20      //�����С
#define CONFIGNUM           7       //���ò�������
#define MAXCROSSNUM         10      //���·������
#define MAXOBJECTNUM        20      //���Ŀ������
#define MAXCAMERANUM        20      //���ڵ�����

//�����ļ������ṹ��
typedef struct FileBufStruct{
	char keyname[BUFSIZE];          //����
	char keyvalue[BUFSIZE];         //��ֵ
}FileBuf;

//�ڵ����ò����ṹ��
typedef struct CameraConfigStruct{
    int nodenum;                    //�ڵ���
    float strengthinit;             //strength��ֵ
    float strengthincre;            //strength����
    float strengthratio;            //ϵ��
    float pvalue;                   //��ʽ�������ʦ�
    float tvalue;                   //��ʽ������ֵ
    MapCoo cameracoo[MAXCAMERANUM]; //�ڵ��ʼ��λ��
}CameraCS;

//Ŀ�����ò����ṹ��
typedef struct ObjectConfigStruct{
    int objectnum;                  //Ŀ����
    MapCoo objectcoo[MAXOBJECTNUM]; //Ŀ���ʼ��λ��
}ObjectCS;

//��ͼ���ò����ṹ��
typedef struct MapConfigStruct{
    int mapx;                       //��ͼx��������
    int mapy;                       //��ͼy��������
    int crossnum;                   //����·����
    MapCoo crosscoo[MAXCROSSNUM];   //·����������
}MapCS;

//ȫ�����ò����ṹ��
typedef struct ConfigParameterStruct{
    CameraCS Camera;
    ObjectCS Object;
    MapCS Map;
}CfgStruct;

extern void OutputControl( Node *node );        //������ƺ���
extern int SystemInit(CfgStruct *cfg);          //ϵͳ��ʼ��
#endif // _GENERAL_H


