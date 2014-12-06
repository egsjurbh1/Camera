#ifndef _READCONFIG_H
#define _READCONFIG_H

#define BUFSIZE             20      //�����С

//�����ļ������ṹ��
typedef struct FileBufStruct{
	char keyname[BUFSIZE];          //����
	char keyvalue[BUFSIZE];         //��ֵ
}FileBuf;

extern int readfile( char *filename, FileBuf *buf);
extern int prasebuffer( const char *keyname, int *location, FileBuf *buf);

#endif // _READCONFIG_H
