#ifndef _READCONFIG_H
#define _READCONFIG_H

#define BUFSIZE             20      //缓存大小

//配置文件解析结构体
typedef struct FileBufStruct{
	char keyname[BUFSIZE];          //键名
	char keyvalue[BUFSIZE];         //键值
}FileBuf;

extern int readfile( char *filename, FileBuf *buf);
extern int prasebuffer( const char *keyname, int *location, FileBuf *buf);

#endif // _READCONFIG_H
