#ifndef _READCONFIG_H
#define _READCONFIG_H

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

extern int readfile( char *filename, FileBuf *buf);
extern int prasebuffer( const char *keyname, int *location, FileBuf *buf);

#endif // _READCONFIG_H
