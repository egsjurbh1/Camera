/** \brief 配置文件读取和解析
 *
 * \author lq
 * \update 141204
 * \return
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <math.h>
#include "readconfig.h"

/**<  File Prase Function
*   out:buf
*/
int readfile( char *filename, FileBuf *buf)
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
        if( c == '#' ) {    //comments
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
*   return 1 success
*/
int prasebuffer( const char *keyname, int *location, FileBuf *buf)
{
    int i,flag;
    i = flag = 0;
    while( buf[i].keyname != NULL)
    {
        if( !strcmp(buf[i].keyname, keyname) ) {
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
